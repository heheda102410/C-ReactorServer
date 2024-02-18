#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"
#include <sys/epoll.h>
#include <string>
using namespace std;
// 声明(不管这个结构体有无被定义出来,先告诉编译器有这么一种类型)
struct EventLoop;
class SelectDispatcher : public Dispatcher {
public:
    SelectDispatcher(EventLoop* evLoop);
    ~SelectDispatcher(); 
    // 添加
    int add() override;
    // 删除
    int remove() override;
    // 修改
    int modify() override;
    // 事件检测
    int dispatch(int timeout = 2) override; // 单位:s
private:
    void setFdSet();
    void clearFdSet();
private:
    fd_set m_readSet;
    fd_set m_writeSet;
    const int m_maxSize = 1024;
};