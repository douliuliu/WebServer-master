/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);                                            // 依据自己响应对象内容向写缓冲区写入响应报文
    void UnmapFile();                                                           // 关闭文件映射
    char* File();                                                               // 返回文件内存映射的指针
    size_t FileLen() const;                                                     // 返回以字节为单位的资源文件容量
    void ErrorContent(Buffer& buff, std::string message);                       // 代表文件不存在，向写缓冲区写入响应体(描述错误的信息)
    int Code() const { return code_; }                                          // 返回状态码

private:
    void AddStateLine_(Buffer &buff);                                           // 向缓冲区写响应首行
    void AddHeader_(Buffer &buff);                                              // 向缓冲区写响应头部
    void AddContent_(Buffer &buff);                                             // 向缓冲区写响应体

    void ErrorHtml_();                                                          // 代表客户端请求错误，返回响应40X页面
    std::string GetFileType_();                                                 // 获得文件类型

    int code_;                                                                  // 响应状态码
    bool isKeepAlive_;                                                          // 是否为长连接

    std::string path_;                                                          // 资源的名称
    std::string srcDir_;                                                        // 资源的目录
    
    char* mmFile_;                                                              // 文件内存映射的指针
    struct stat mmFileStat_;                                                    // 资源文件的状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;      // map,键为文件后缀名,值为文件类型
    static const std::unordered_map<int, std::string> CODE_STATUS;              // map,键为状态码,值为状态描述
    static const std::unordered_map<int, std::string> CODE_PATH;                // map,键为状态码,值为资源名称
};

#endif //HTTP_RESPONSE_H