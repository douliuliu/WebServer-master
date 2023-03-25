/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;                  // 回调函数
typedef std::chrono::high_resolution_clock Clock;               // 高精度时钟
typedef std::chrono::milliseconds MS;                           // 毫秒
typedef Clock::time_point TimeStamp;                            // 时间戳

struct TimerNode {                                              // 定时器结构体
    int id;                                                     // 文件描述符
    TimeStamp expires;                                          // 超时时间
    TimeoutCallBack cb;                                         // 超时回调函数
    bool operator<(const TimerNode& t) {                        // 重载小于运算符
        return expires < t.expires;
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }                          // 预留64个定时器内存

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);                        // 发生数据交流，设定新的超时时间，所以需要调整堆

    void add(int id, int timeOut, const TimeoutCallBack& cb);   // 加一个定时器

    void doWork(int id);

    void clear();                                               // 清空heap_和ref_

    void tick();                                                // 清除超时结点

    void pop();                                                 // 清除堆中第一个定时器，即最先超时的定时器

    int GetNextTick();                                          // 清除超时结点，并返回到下一个超时节点的时间差

private:
    void del_(size_t i);                                        // 删除一个定时器，i是索引
    
    void siftup_(size_t i);                                     // 将一个节点向上调整，i是索引

    bool siftdown_(size_t index, size_t n);                     // 将一个节点向下调整，n是heap_长度

    void SwapNode_(size_t i, size_t j);                         // 交换两个定时器

    std::vector<TimerNode> heap_;                               // 放定时器的容器

    std::unordered_map<int, size_t> ref_;                       // 键是文件描述符，值是索引
};

#endif //HEAP_TIMER_H