#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"
#include <sys/epoll.h>
#include <string>
using namespace std;

// 声明(不管这个结构体有无被定义出来,先告诉编译器有这么一种类型)
struct EventLoop;
class EpollDispatcher : public Dispatcher {
public:
    EpollDispatcher(EventLoop* evLoop);
    ~EpollDispatcher(); 
    // 添加
    int add() override;
    // 删除
    int remove() override;
    // 修改
    int modify() override;
    // 事件检测
    int dispatch(int timeout = 2) override; // 单位:s
private:
    int epollCtl(int op);
private:
    int m_epfd;//epoll树的根节点
    struct epoll_event* m_events;// 数组指针
    const int m_maxNode = 520;
};