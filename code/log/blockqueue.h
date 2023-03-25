/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t MaxCapacity = 1000); // 最大容量1000

    ~BlockDeque();

    void clear();                                   // 清空队列

    bool empty();                                   // 判断队列是否为空

    bool full();                                    // 判断队列是否为满

    void Close();                                   // 清空队列，关闭子线程

    size_t size();                                  // 返回队列当前大小

    size_t capacity();                              // 返回队列总容量

    T front();                                      // 返回队列的第一个元素

    T back();                                       // 返回队列的最后一个元素

    void push_back(const T &item);                  // 队列尾部添加一个元素，并唤醒一个消费者

    void push_front(const T &item);                 // 队列头部添加一个元素，并唤醒一个消费者

    bool pop(T &item);                              // 唤醒一个消费者

    bool pop(T &item, int timeout);                 // 唤醒一个消费者

    void flush();                                   // 唤醒一个消费者

private:
    std::deque<T> deq_;                             // 存日志信息的队列，每一个元素代表一个日志信息

    size_t capacity_;                               // 队列的最大容量

    std::mutex mtx_;                                // 互斥锁

    bool isClose_;                                  // 子线程是否关闭的标志

    std::condition_variable condConsumer_;          // 消费者条件变量

    std::condition_variable condProducer_;          // 生产者条件变量
};


template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);
    isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

// 清空队列，关闭子线程
template<class T>
void BlockDeque<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
        isClose_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
};

// 唤醒一个子线程从日志队列取日志数据写入日志文件中
template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();
};

// 清空队列
template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

// 返回队列的第一个元素
template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

// 返回队列的最后一个元素
template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

// 返回队列当前大小
template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

// 返回队列总容量
template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

// 队列尾部添加一个日志信息，并唤醒一个在pop()中阻塞的子线程
template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker); 
    }
    deq_.push_back(item);
    condConsumer_.notify_one();     // 如果没有子线程在该条件变量阻塞，则不执行任何操作并返回
}

// 队列头部添加一个日志信息，并唤醒一个在pop()中阻塞的子线程
template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

// 判断队列是否为空
template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

// 判断队列是否为满
template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

// 当日志信息队列为空需要wait阻塞，等待Log::write函数中调用，isClose_为真返回false关闭子线程，
// 从队列取出一个日志信息引用传值给参数item，日志信息队列大小减一，并唤醒一个生产者
template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        condConsumer_.wait(locker);
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

// 从队列取出一个日志信息引用传值给参数item，日志信息队列大小减一，并唤醒一个生产者
template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout){
            return false;
        }
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

#endif // BLOCKQUEUE_H