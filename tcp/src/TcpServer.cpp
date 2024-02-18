#include "TcpServer.h"
#include "TcpConnection.h"
#include <arpa/inet.h> // 套接字函数的头文件
#include <stdio.h>
#include <stdlib.h>
#include "Log.h"

int TcpServer::acceptConnection(void* arg) {
    TcpServer* server = static_cast<TcpServer*>(arg);
    // 和客户端建立连接
    int cfd = accept(server->m_lfd,nullptr,nullptr);
    // 从线程池中去取出一个子线程的反应堆实例，去处理这个cfd
    EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();
    // 将cfd放到 TcpConnection中处理
    new TcpConnection(cfd, evLoop);
    return 0;
}

TcpServer::TcpServer(unsigned short port,int threadNum) {
    m_port = port;
    m_mainLoop = new EventLoop; // 主线程的事件循环（反应堆模型）
    m_threadPool = new ThreadPool(m_mainLoop,threadNum); // 创建线程池
    m_threadNum = threadNum; // 线程数量
    setListen(); // 初始化监听
}

TcpServer::~TcpServer() { // 可以不写

}

// 初始化监听
void TcpServer::setListen() {
    // 1.创建一个监听的文件描述符 -> m_lfd
    m_lfd = socket(AF_INET,SOCK_STREAM,0); // AF_INET -> （网络层协议：Ipv4） ；SOCK_STREAM -> （传输层协议：流式协议）  ；0 -> :表示使用Tcp
    if(m_lfd == -1) {
        perror("socket");              
        return;
    }   
    // 2.设置端口复用
    int opt = 1;
    int ret =  setsockopt(m_lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)); 
    if(ret == -1) {
        perror("setsockopt");
        return;
    }
    // 3.绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);// 主机字节序（小端）转成网络字节序（大端） 端口的最大数量：2^16=65536
    addr.sin_addr.s_addr = INADDR_ANY;// 0.0.0.0 
    ret = bind(m_lfd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret == -1) {
        perror("bind");
        return;
    }
    // 4.设置监听
    ret = listen(m_lfd,128);
    if(ret == -1) {
        perror("listen");
        return;
    }
}

// 启动服务器（不停检测有无客户端连接）
void TcpServer::run() { 
    Debug("服务器程序已经启动了...");
    // 启动线程池
    m_threadPool->run();
    // 初始化一个channel实例
    Channel* channel = new Channel(m_lfd,FDEvent::ReadEvent,acceptConnection,nullptr,nullptr,this);
    // 添加检测的任务 
    m_mainLoop->addTask(channel,ElemType::ADD);
    // 启动反应堆模型
    m_mainLoop->run();
}
