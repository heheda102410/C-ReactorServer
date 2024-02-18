#pragma once
#include "Buffer.h"
#include <stdbool.h>
#include "HttpResponse.h"
#include <map>
using namespace std;

// 当前的解析状态
enum class PrecessState:char {
    ParseReqLine,    // 当前解析的是请求行
    ParseReqHeaders, // 当前解析的是请求头
    ParseReqBody,    // 当前解析的是请求体
    ParseReqDone     // 完成解析
};

// 定义http请求结构体
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
    // 重置
    void reset();
    // 添加请求头
    void addHeader(const string key, const string value);
    // 根据key得到请求头的value
    string getHeader(const string key);
    // 解析请求行
    bool parseRequestLine(Buffer* readBuf);
    // 解析请求头
    bool parseRequestHeader(Buffer* readBuf);
    // 解析http请求协议
    bool parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
    // 处理http请求协议
    bool processHttpRequest(HttpResponse* response);
    
    // 发送文件
    static void sendFile(string dirName, Buffer* sendBuf, int cfd);
    // 判断文件扩展名，返回对应的 Content-Type(Mime-Type)
    const string getFileType(const string name);

    // 发送目录
    static void sendDir(string dirName, Buffer* sendBuf, int cfd);
    // 解码字符串
    string decodeMsg(string from);

    // 获取处理状态
    inline PrecessState getState() {
        return m_curState;
    }
    // 更新处理状态
    inline void setState(PrecessState state) {
        m_curState = state;
    }
    
    // 更新请求方法
    inline void setMethod(string method) {
        m_method = method;
    }
    // 更新Url
    inline void seturl(string url) {
        m_url = url;
    }
    // 更新协议版本
    inline void setVersion(string version) {
        m_version = version;
    }
private:
    char* splitRequestLine(const char* start, const char* end,
        const char* sub, function<void(string)> callback);
    int hexToDec(char c);

private:
    // 当前解析状态
    PrecessState m_curState;
    // 请求行
    string m_method;
    string m_url;
    string m_version;
    // 请求头
    map<string, string> m_reqHeaders;
};

