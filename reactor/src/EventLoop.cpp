#include "EventLoop.h"
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SelectDispatcher.h"
#include "PollDispatcher.h"
#include "EpollDispatcher.h"

// 写数据
void EventLoop::taskWakeup() {
    const char* msg = "我是要成为海贼王的男人！";
    write(m_socketPair[0],msg,strlen(msg));
}

// 读数据
int EventLoop::readLocalMessage(void* arg) {
    EventLoop* evLoop = static_cast<EventLoop*>(arg);
    char buf[256];
    read(evLoop->m_socketPair[1],buf,sizeof(buf));
    return 0;
}

/*  
    访问类的私有成员就是访问当前类的实例里边的私有成员,
    因为类里边的非静态成员函数和非静态成员变量都是属于
    对象的.
*/
// 读数据
int EventLoop::readMessage() {
    char buf[256];
    read(m_socketPair[1], buf, sizeof(buf));
    return 0;
}

// 委托构造函数
EventLoop::EventLoop() : EventLoop(string()) {

}

EventLoop::EventLoop(const string threadName) {
    m_isQuit = true; // 默认没有启动
    // m_dispatcher = new EpollDispatcher(this);
    // m_dispatcher = new SelectDispatcher(this);
    m_dispatcher = new PollDispatcher(this);
    // map
    m_channelMap.clear();
    m_threadID = this_thread::get_id(); // 当前线程ID
    m_threadName = threadName == string() ? "MainThread" : threadName; // 线程的名字
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);
    if(ret == -1) {
        perror("socketpair");
        exit(0);
    }
#if 0
    // 指定规则：m_socketPair[0] 发送数据，m_socketPair[1]接收数据
    Channel* channel = new Channel(m_socketPair[1],FDEvent::ReadEvent, 
        readLocalMessage,nullptr,nullptr,this);
#else 
    // 绑定 - bind(可调用对象绑定器)
    auto obj = bind(&EventLoop::readMessage,this);
    Channel* channel = new Channel(m_socketPair[1],FDEvent::ReadEvent, 
        obj,nullptr,nullptr,this);
    /*
        使用可调用对象包装器不能够直接对类的非静态成员函数进行打包,
        既然不能直接打包,就需要间接的来完成这些操作
        可以通过bind(可调用对象绑定器),先来做一个绑定,绑定好了之后
        得到obj可调用对象,就可以通过function来进行打包了
    */
#endif
    // channel 添加到任务队列
    addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop() {

}

// 启动反应堆模型
int EventLoop::run() {
    m_isQuit = false; // 不退出
    // 比较线程ID是否正常
    if(m_threadID != this_thread::get_id()) {
        return -1;
    }
    // 循环进行事件处理
    while(!m_isQuit) {
        m_dispatcher->dispatch(); // 默认设置超时时长 2s
        processTaskQ();
    }
    return 0;  
}

// 处理被激活的文件描述符fd
int EventLoop::eventActive(int fd,int event) {
    if (fd < 0) {
        return -1;
    }
    // 取出channel
    Channel* channel = m_channelMap[fd];
    assert(channel->getSocket() == fd);
    if (event & (int)FDEvent::ReadEvent && channel->readCallback) {
        channel->readCallback(const_cast<void*>(channel->getArg()));
    }
    if (event & (int)FDEvent::WriteEvent && channel->writeCallback) {
        channel->writeCallback(const_cast<void*>(channel->getArg()));
    }
    return 0;
}

// 添加任务到任务队列
int EventLoop::addTask(Channel* channel, ElemType type) {
    // 加锁, 保护共享资源
    m_mutex.lock();
    // 创建新节点
    ChannelElement* node = new ChannelElement;
    node->channel = channel;
    node->type = type;
    m_taskQ.push(node);
    m_mutex.unlock();
    // 处理节点
    /*
    * 细节:
    *   1. 对于链表节点的添加: 可能是当前线程也可能是其他线程(主线程)
    *       1). 修改fd的事件, 当前子线程发起, 当前子线程处理
    *       2). 添加新的fd, 添加任务节点的操作是由主线程发起的
    *   2. 不能让主线程处理任务队列, 需要由当前的子线程取处理
    */
    if (m_threadID == this_thread::get_id())
    {
        // 当前子线程(基于子线程的角度分析)
        processTaskQ();
    }
    else
    {
        // 主线程 -- 告诉子线程处理任务队列中的任务
        // 1. 子线程在工作 2. 子线程被阻塞了:select, poll, epoll
        taskWakeup();
    }
    return 0;
}

// 处理任务队列中的任务
int EventLoop::processTaskQ() {
    // 取出头节点
    while (!m_taskQ.empty()) {
        m_mutex.lock();
        ChannelElement* node = m_taskQ.front();
        m_taskQ.pop();// 删除节点
        m_mutex.unlock();
        Channel* channel = node->channel;
        if(node->type == ElemType::ADD) {
            // 添加
            add(channel);
        }
        else if(node->type == ElemType::DELETE) {
            // 删除
            remove(channel);
        }
        else if(node->type == ElemType::MODIFY) {
            // 修改
            modify(channel);
        }
        // 释放节点
        delete node;
    }
    return 0;
}

// 处理dispatcher中的任务
int EventLoop::add(Channel* channel) {
    int fd = channel->getSocket();// 取出文件描述符fd
    // 找到fd对应的数组元素位置，并存储
    if(m_channelMap.find(fd) == m_channelMap.end()) {
        m_channelMap.insert(make_pair(fd,channel));
        m_dispatcher->setChannel(channel);
        int ret = m_dispatcher->add();
        return ret;
    } 
    return -1;
}

int EventLoop::remove(Channel* channel) {
    int fd = channel->getSocket();
    if(m_channelMap.find(fd) == m_channelMap.end()) {
        return -1;
    }
    // 如果文件描述符fd在检测集合里，就从中把它删除
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->remove();
    return ret;
}

int EventLoop::modify(Channel* channel) {
    int fd = channel->getSocket();
    if (m_channelMap.find(fd) == m_channelMap.end())
    {
        return -1;
    }
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->modify();
    return ret;
}

// 释放channel
int EventLoop::freeChannel(Channel* channel) {
    // 删除 channel 和 fd 的对应关系
    auto it = m_channelMap.find(channel->getSocket());
    if(it != m_channelMap.end()) {
        m_channelMap.erase(it);
        close(channel->getSocket());// 关闭文件描述符
        delete channel;// 释放 channel 内存
    }
    return 0;
}

