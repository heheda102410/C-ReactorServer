#include "HttpResponse.h"
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ResHeaderSize 16

HttpResponse::HttpResponse() {
    // 状态行:状态码，状态描述
    m_statusCode = StatusCode::Unknown;
    // 响应头 - 键值对
    m_headers.clear();
    // 文件名
    m_fileName = string();
    // 不是函数指针,而是可调用对象包装器的对象
    sendDataFunc = nullptr;
}

HttpResponse::~HttpResponse() {

}

// 添加响应头
void HttpResponse::addHeader(const string key,const string value) {
    if(key.empty() || value.empty()) {
        return;
    }
    m_headers.insert(make_pair(key,value));
}

// 组织http响应数据
void HttpResponse::prepareMsg(Buffer* sendBuf,int socket) {
    // 状态行
    char tmp[1024] = {0};
    int code = static_cast<int>(m_statusCode);
    // 注意[]和at的语法,at可用于只读情况
    sprintf(tmp,"HTTP/1.1 %d %s\r\n",code,m_info.at(code).data());
    sendBuf->appendString(tmp);
    
    // 响应头
    for(auto it=m_headers.begin();it!=m_headers.end();++it) {
        sprintf(tmp,"%s: %s\r\n",it->first.data(),it->second.data());
        sendBuf->appendString(tmp);
    }
    // 空行
    sendBuf->appendString("\r\n");
    
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(socket);
#endif
    // 回复的数据
    sendDataFunc(m_fileName.data(),sendBuf,socket);
}