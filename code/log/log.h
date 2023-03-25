/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
public:
    void init(int level, const char* path = "./log",        // 初始化日志对象
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* Instance();                                 // 取得单例模式的实例
    static void FlushLogThread();                           // 调用AsyncWrite_()异步写日志
    // (如果开启异步方式)先向自己缓冲区写入日志信息，然后将缓冲区的日志数据封装成一个string字符串加入日志队列，由子线程取出写到文件中
    void write(int level, const char *format,...);
    // 唤醒一个子线程，把可能还在日志队列中的信息取出输出到文件中，调用fflush(stream)会将标准缓冲区中的内容清空并写到stream所指的文件中去
    void flush();

    int GetLevel();                                         // 获得日志等级
    void SetLevel(int level);                               // 设置日志等级
    bool IsOpen() { return isOpen_; }                       // 日志是否打开
    
private:
    Log();
    void AppendLogLevelTitle_(int level);                   // 追加日志等级标签
    virtual ~Log();
    void AsyncWrite_();                                     // 异步写日志

private:
    static const int LOG_PATH_LEN = 256;                    // 日志路径的长度
    static const int LOG_NAME_LEN = 256;                    // 日志文件的长度
    static const int MAX_LINES = 50000;                     // 一个日志文件最多装多少行数据

    const char* path_;                                      // 日志路径
    const char* suffix_;                                    // 前缀

    int MAX_LINES_;                                         // 一个日志文件最多装多少行数据

    int lineCount_;                                         // 当前日志文件写了多少行
    int toDay_;                                             // 今天日期

    bool isOpen_;                                           // 日志是否打开
 
    Buffer buff_;                                           // 存日志数据的缓冲区
    int level_;                                             // 日志级别
    bool isAsync_;                                          // 日志是否异步的标志

    FILE* fp_;                                              // 操控日志文件的指针
    std::unique_ptr<BlockDeque<std::string>> deque_;        // 阻塞队列的指针
    std::unique_ptr<std::thread> writeThread_;              // 执行写日志的子线程指针
    std::mutex mtx_;                                        // 互斥锁
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H