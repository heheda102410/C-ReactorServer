#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>

ThreadPool::ThreadPool(EventLoop* mainLoop, int threadNum) {
    m_mainLoop = mainLoop; // 主线程的反应堆模型
    m_index = 0;
    m_isStart = false;
    m_threadNum = threadNum; // 子线程总个数
    m_workerThreads.clear();// 子线程数组
}

ThreadPool::~ThreadPool() {
    for(auto item : m_workerThreads) {
        delete item;
    }
}

// 启动线程池 
void ThreadPool::run() {
    // 确保线程池未运行  
    assert(!m_isStart);
    // 比较主线程的ID和当前线程ID是否相等 
    // 相等=>确保执行线程为主线程；不相等=>exit(0)
    if (m_mainLoop->getThreadID() != this_thread::get_id()) {
        exit(0);
    }
    m_isStart = true; // 标记为启动
    if(m_threadNum > 0) { // 线程数量大于零
        for(int i=0;i<m_threadNum;++i) {
            WorkerThread* subThread = new WorkerThread(i); // 子线程实例
            subThread->run();
            m_workerThreads.push_back(subThread); // 子线程数组
        }
    }
}

// 取出线程池中的某个子线程的反应堆实例
EventLoop* ThreadPool::takeWorkerEventLoop() {
    assert(m_isStart); // 确保线程池是运行的
    // 比较主线程的ID和当前线程ID是否相等 
    // 相等=>确保执行线程为主线程；不相等=>exit(0)
    if (m_mainLoop->getThreadID() != this_thread::get_id()) { 
        exit(0);
    }
    // 从线程池中找到一个子线程，然后取出里边的反应堆实例
    EventLoop* evLoop = m_mainLoop; // 初始化
    if(m_threadNum > 0) { // 线程数量大于零
        evLoop = m_workerThreads[m_index]->getEventLoop();
        m_index = ++m_index % m_threadNum;
    }
    return evLoop;
}
