#pragma once
#include "EventLoop.h"
#include <stdbool.h>
#include "WorkerThread.h"
#include <vector>
using namespace std;

// 定义线程池
class ThreadPool {
public:
    ThreadPool(EventLoop* mainLoop, int threadNum);
    ~ThreadPool();
    // 启动线程池 
    void run();
    // 取出线程池中的某个子线程的反应堆实例
    EventLoop* takeWorkerEventLoop();
private:
    EventLoop* m_mainLoop; // 主线程的反应堆模型
    int m_index; 
    bool m_isStart;
    int m_threadNum; // 子线程总个数
    vector<WorkerThread*> m_workerThreads;
};

