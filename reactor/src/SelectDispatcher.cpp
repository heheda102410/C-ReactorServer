#include "SelectDispatcher.h"
#include "Dispatcher.h"
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
 
void SelectDispatcher::setFdSet() {
    if (m_channel->getEvent() & (int)FDEvent::ReadEvent) {
        FD_SET(m_channel->getSocket(),&m_readSet);
    }
    if (m_channel->getEvent() & (int)FDEvent::WriteEvent) {
        FD_SET(m_channel->getSocket(),&m_writeSet);
    }
}
 
void SelectDispatcher::clearFdSet() {
    if (m_channel->getEvent() & (int)FDEvent::ReadEvent) {
        FD_CLR(m_channel->getSocket(),&m_readSet);
    }
    if (m_channel->getEvent() & (int)FDEvent::WriteEvent) {
        FD_CLR(m_channel->getSocket(),&m_writeSet);
    }
}

// 在调用子类的构造函数还需要调用父类的构造函数,evLoop实例的保存其实是在父类的构造函数里边做的
// 如果没有这样做,不能指向一块有效的内存
SelectDispatcher::SelectDispatcher(EventLoop* evLoop) : Dispatcher(evLoop){
    FD_ZERO(&m_readSet);
    FD_ZERO(&m_writeSet);
    m_name = "Select";
}

SelectDispatcher::~SelectDispatcher() {
    
} 

// 添加
int SelectDispatcher::add() {
    if(m_channel->getSocket() >= m_maxSize) {
        return -1;
    }
    setFdSet();
    return 0;
}

// 删除
int SelectDispatcher::remove() {
    clearFdSet();
    // 通过 m_channel 释放对应的 TcpConnection 资源
    m_channel->destroyCallback(const_cast<void*>(m_channel->getArg())); // (已续写)
    return 0;
} 

// 修改
int SelectDispatcher::modify() {
    setFdSet();
    clearFdSet();
    return 0;
} 

// 事件检测
int SelectDispatcher::dispatch(int timeout) {
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set rdtmp = m_readSet;
    fd_set wrtmp = m_writeSet;
    int count = select(m_maxSize,&rdtmp,&wrtmp,NULL,&val);
    if(count == -1) {
        perror("select");
        exit(0);
    }
    for(int i=0;i<m_maxSize;++i) { 
        if(FD_ISSET(i,&rdtmp)) {
            m_evLoop->eventActive(i,(int)FDEvent::ReadEvent); 
        }
        if(FD_ISSET(i,&wrtmp)) {
            m_evLoop->eventActive(i,(int)FDEvent::WriteEvent); 
        }
    }
    return 0;
} 
