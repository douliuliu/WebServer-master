/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest {
public:
    enum PARSE_STATE {                                      // 主状态机状态的可能情况
        REQUEST_LINE,                                       // 解析请求行
        HEADERS,                                            // 解析请求头部
        BODY,                                               // 解析请求体
        FINISH,                                             // 完成解析
    };

    enum HTTP_CODE {                                        // 响应结果的可能情况
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();                                            // 初始化请求对象
    bool parse(Buffer& buff);                               // 从buff缓冲区中解析http请求

    std::string path() const;                               // 获得请求资源路径
    std::string& path();
    std::string method() const;                             // 获得请求方法
    std::string version() const;                            // 获得请求http版本
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;                               // 是否保持http长连接

    /* 
    todo 
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    bool ParseRequestLine_(const std::string& line);        // 解析请求行
    void ParseHeader_(const std::string& line);             // 解析请求头部
    void ParseBody_(const std::string& line);               // 解析请求体

    void ParsePath_();                                      // 解析资源路径
    void ParsePost_();                                      // 解析用户名密码并验证登陆
    void ParseFromUrlencoded_();                            // 解析用户名密码
    // 验证用户信息
    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;                                     // 主状态机解析状态
    std::string method_;                                    // 请求方法
    std::string path_;                                      // 请求资源名称
    std::string version_;                                   // 请求http版本
    std::string body_;                                      // 请求体内容
    std::unordered_map<std::string, std::string> header_;   // 请求头部map，键是请求头部名称，值是请求头部的具体值
    std::unordered_map<std::string, std::string> post_;     // 用户名密码map
    // 默认的html资源名，不带文件后缀名
    static const std::unordered_set<std::string> DEFAULT_HTML;
    // html资源名，带文件后缀名map， 键是html资源名，值是序号
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);                          // 密码进行16进制加密
};


#endif //HTTP_REQUEST_H