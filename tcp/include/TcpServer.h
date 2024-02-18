#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

class TcpServer {
public:
    TcpServer(unsigned short port,int threadNum);
    ~TcpServer();
    // 初始化监听
    void setListen();
    // 启动服务器（不停检测有无客户端连接）
    void run();
private:
    static int acceptConnection(void* arg);
private:
    EventLoop* m_mainLoop; // 主线程的事件循环（反应堆模型）
    ThreadPool* m_threadPool; // 线程池
    int m_threadNum; // 线程数量
    int m_lfd;
    unsigned short m_port;
};

