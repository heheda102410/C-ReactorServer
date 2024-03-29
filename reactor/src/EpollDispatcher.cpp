#include "EpollDispatcher.h"
#include "Dispatcher.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
 
int EpollDispatcher::epollCtl(int op) {
    struct epoll_event ev;
    // 先把要检测的文件描述符存储到类型为epoll_event 的ev中
    ev.data.fd = m_channel->getSocket();
    // 指定要检测的这个fd的事件
    int events = 0;
    if(m_channel->getEvent() & (int)FDEvent::ReadEvent) {
        events |= EPOLLIN;
    }
    if(m_channel->getEvent() & (int)FDEvent::WriteEvent) {
        events |= EPOLLOUT;
    }
    ev.events = events;
    int ret = epoll_ctl(m_epfd,op,m_channel->getSocket(),&ev);
    return ret;
}

EpollDispatcher::EpollDispatcher(EventLoop* evLoop) : Dispatcher(evLoop){
    m_epfd = epoll_create(10);
    if(m_epfd == -1) {
        perror("epoll_create");
        exit(0);
    }
    m_events = new struct epoll_event[m_maxNode];
    m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher() {
    close(m_epfd);
    delete[] m_events;
    m_events = nullptr;
}

// 添加
int EpollDispatcher::add() {
    int ret = epollCtl(EPOLL_CTL_ADD);
    if(ret == -1) {
        perror("epoll_ctl add");
        exit(0);
    }
    return ret;
}

// 删除
int EpollDispatcher::remove() {
    int ret = epollCtl(EPOLL_CTL_DEL);
    if(ret == -1) {
        perror("epoll_ctl delete");
        exit(0);
    } 
    // 通过 m_channel 释放对应的 TcpConnection 资源
    // const_cast去掉只读属性
    m_channel->destroyCallback(const_cast<void*>(m_channel->getArg())); // (已续写)
    return ret;
}

// 修改
int EpollDispatcher::modify() {
    int ret = epollCtl(EPOLL_CTL_MOD);
    if(ret == -1) {
        perror("epoll_ctl modify");
        exit(0);
    }
    return ret;
}

// 事件检测
int EpollDispatcher::dispatch(int timeout) {
    int count = epoll_wait(m_epfd,m_events,m_maxNode,timeout * 1000);
    for(int i=0;i<count;++i) {
        int events = m_events[i].events;
        int fd = m_events[i].data.fd;
        if(events & EPOLLERR || events & EPOLLHUP) {
            // 对方断开了连接，删除 fd
            // epollRemove(Channel, evLoop);
            continue;
        }
        if(events & EPOLLIN) {
            m_evLoop->eventActive(fd, (int)FDEvent::ReadEvent);
        }
        if(events & EPOLLOUT) {
            m_evLoop->eventActive(fd, (int)FDEvent::WriteEvent);
        }
    }
    return 0;
}