#pragma once
#include "Dispatcher.h"
#include "Channel.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <string>
using namespace std;

// 处理该节点中的channel的方式
enum class ElemType:char{
    ADD,    // 添加
    DELETE, // 删除
    MODIFY  // 修改
};

// 定义任务队列的节点
struct ChannelElement {
    ElemType type;// 如何处理该节点中的channel
    Channel* channel;
};

// 声明(不管这个结构体有无被定义出来,先告诉编译器有这么一种类型)
class Dispatcher;

class EventLoop{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();
    // 启动反应堆模型
    int run();
    // 处理被激活的文件描述符fd
    int eventActive(int fd,int event);
    // 添加任务到任务队列
    int addTask(Channel* channel,ElemType type);
    // 处理任务队列中的任务
    int processTaskQ();
    // 处理dispatcher中的任务
    int add(Channel* channel);
    int remove(Channel* channel);
    int modify(Channel* channel);
    // 释放channel
    int freeChannel(Channel* channel);
    static int readLocalMessage(void* arg);
    int readMessage();// 类的成员函数就不需要给它传递参数了
    // 返回线程ID
    inline thread::id getThreadID() {
        return m_threadID;
    }
    inline string getThreadName()
    {
        return m_threadName;
    }
private:
    void taskWakeup();
private:
    bool m_isQuit;// 开关
    // 该指针指向子类的实例 epoll,poll,select
    Dispatcher* m_dispatcher;
    // 任务队列
    queue<ChannelElement*>m_taskQ;
    // 用于存储channel的map
    map<int,Channel*> m_channelMap;
    // 线程ID，Name，mutex
    thread::id m_threadID;
    string m_threadName;
    mutex m_mutex;
    int m_socketPair[2]; //存储本地通信的fd 通过socketpair初始化
};
