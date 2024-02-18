#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include <string>
using namespace std;
// 声明(不管这个结构体有无被定义出来,先告诉编译器有这么一种类型)
class EventLoop;
class Dispatcher {
public:
    Dispatcher(EventLoop* evLoop);
    virtual ~Dispatcher(); 
    // 添加
    virtual int add();
    // 删除
    virtual int remove();
    // 修改
    virtual int modify();
    // 事件检测
    virtual int dispatch(int timeout = 2); // 单位:s
    inline void setChannel(Channel* channel) { 
        m_channel = channel; 
    }
protected:
    string m_name = string();
    Channel* m_channel;
    EventLoop* m_evLoop;
};