/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);     // 初始化一个客户端通信连接

    ssize_t read(int* saveErrno);                       // 子线程从内核缓冲区读数据到Buffer readBuff_的读缓冲区

    ssize_t write(int* saveErrno);                      // 子线程从Buffer writeBuff_的写缓冲区写数据到内核缓冲区

    void Close();                                       // 关闭与客户端连接

    int GetFd() const;                                  // 得到通信文件描述符

    int GetPort() const;                                // 得到客户端的端口

    const char* GetIP() const;                          // 到客户端的ip地址
    
    sockaddr_in GetAddr() const;                        // 到客户端的ip地址和端口
    
    bool process();                                     // 业务处理（解析请求，返回响应）

    // 分散写块中还需要写的的字节长度
    int ToWriteBytes() {                                
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const {                          // 是否保持连接
        return request_.IsKeepAlive();
    }

    static bool isET;                                   // 是否是ET模式
    static const char* srcDir;                          // 资源的目录
    static std::atomic<int> userCount;                  // 总共的客户端的连接数
    
private:
   
    int fd_;                                            // 与客户端通信的描述符
    struct  sockaddr_in addr_;                          // 客户端的地址信息

    bool isClose_;                                      // 是否关闭连接标志
    
    int iovCnt_;                                        // 分散写块数
    struct iovec iov_[2];                               // 分散写数组
    
    Buffer readBuff_;                                   // 读缓冲区
    Buffer writeBuff_;                                  // 写缓冲区

    HttpRequest request_;                               // 请求对象
    HttpResponse response_;                             // 响应对象
};

#endif //HTTP_CONN_H