#include "WorkerThread.h"
#include <stdio.h>

// 子线程的回调函数
void WorkerThread::running() {
    // 还有子线程里边的evLoop是共享资源，需要添加互斥锁
    m_mutex.lock();
    m_evLoop = new EventLoop(m_name);
    m_mutex.unlock();// 解锁
    m_cond.notify_one();// 发送信号（唤醒主线程，通知主线程解除阻塞）
    m_evLoop->run();// 启动反应堆模型
}

WorkerThread::WorkerThread(int index)
{
    m_evLoop = nullptr;
    m_thread = nullptr;
    m_threadID = thread::id();
    m_name =  "SubThread-" + to_string(index);
}

WorkerThread::~WorkerThread()
{
    if (m_thread != nullptr)
    {
        delete m_thread;
    }
}

// 启动线程
void WorkerThread::run()
{
    // 创建子线程
    m_thread = new thread(&WorkerThread::running, this);
    // 阻塞主线程, 让当前函数不会直接结束
    unique_lock<mutex> locker(m_mutex);
    while (m_evLoop == nullptr)
    {
        m_cond.wait(locker);
    }
}