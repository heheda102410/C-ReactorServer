#pragma once
#include <thread>
#include "EventLoop.h"
#include <mutex>
#include <condition_variable>
#include <string>
using namespace std;
// 定义子线程对应的结构体
class WorkerThread {
public:
    WorkerThread(int index);
    ~WorkerThread();
    // 启动线程
    void run();
    inline EventLoop* getEventLoop() {
        return m_evLoop;
    }
private:
    void running();
private:
    thread* m_thread;// 保存线程的实例
    thread::id m_threadID;// 线程ID
    string m_name;// 线程名字
    mutex m_mutex;// 互斥锁（线程同步）
    condition_variable m_cond;// 条件变量（线程阻塞）
    EventLoop* m_evLoop;// 事件循环(反应堆模型) 在每个子线程里边都有一个反应堆模型
};

