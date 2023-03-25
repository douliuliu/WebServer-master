/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool* Instance();                 // 单例模式中的懒汉模式

    MYSQL* GetConn();                               // 从连接池队列获得一个可用mysql连接
    void FreeConn(MYSQL* conn);                     // 释放一个连接，放入到队列里，注意不是关闭连接，而是释放掉重新用
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);    // 初始化mysql连接池，默认10个连接，放入connQue_可用数据库连接队列中，信号量开始为10
    void ClosePool();                               // 关闭数据库连接池，即把所有的数据库连接都关闭了

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;                                  // 最大数据库连接数
    int useCount_;                                  // 已经使用数据库连接数
    int freeCount_;                                 // 空闲的数据库连接数

    std::queue<MYSQL *> connQue_;                   // 可用数据库连接队列
    std::mutex mtx_;                                // 互斥锁
    sem_t semId_;                                   // 信号量
};


#endif // SQLCONNPOOL_H