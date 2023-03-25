/*
    Author       : liudou
    Date         : 2022-12-24
*/
#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);                // 缓冲区默认大小为1k
    ~Buffer() = default;

    size_t WritableBytes() const;                   // 缓冲区中还可以写的字节数量
    size_t ReadableBytes() const;                   // 缓冲区中还需要读的字节数量
    size_t PrependableBytes() const;                // 缓冲区中已经读完字节数量，这部分内存可以再次利用

    const char* Peek() const;                       // 读的位置的地址
    void EnsureWriteable(size_t len);               // 保证有缓冲区空间可写
    void HasWritten(size_t len);                    // 已经写了len长度字节，将writePos_（写的位置）加len

    void Retrieve(size_t len);                      // 将读位置加上http请求报文一行的长度
    void RetrieveUntil(const char* end);            // 保证读位置小于下一行位置

    void RetrieveAll();                             // 将缓冲区（清空)全置为0，读写位置变为开头
    std::string RetrieveAllToStr();                 // 以缓冲区所有还需要读的字符构造一个string字符串，然后清空缓冲区并返回该字符串

    const char* BeginWriteConst() const;            // 同下一行
    char* BeginWrite();                             // 写的位置的地址

    void Append(const std::string& str);            // 从str地址头开始，将str所代表的字符串写入缓冲区
    void Append(const char* str, size_t len);       // 从str地址头开始，将len长度的字节写入缓冲区,保证有可以用的空间，且从writePos_即写的位置开始写
    void Append(const void* data, size_t len);      // 从data转为字符类型指针的地址头开始，将len长度的字节写入缓冲区
    void Append(const Buffer& buff);                // 将参数里缓冲区的所有可读字节写入到this指针所指对象的缓冲区中

    ssize_t ReadFd(int fd, int* Errno);             // 读取客户端的数据，将文件描述符的内核缓冲区数据读到我们的读缓冲区中
    ssize_t WriteFd(int fd, int* Errno);            // 输出响应的数据，将我们的写缓冲区中的响应数据写到文件描述符的内核缓冲区

private:
    char* BeginPtr_();                              // buffer_缓冲区开始地址
    const char* BeginPtr_() const;                  // buffer_缓冲区开始地址
    void MakeSpace_(size_t len);                    // 得到额外可读(写)空间

    std::vector<char> buffer_;                      // 缓冲区即具体装数据的容器
    std::atomic<std::size_t> readPos_;              // 读的位置
    std::atomic<std::size_t> writePos_;             // 写的位置
};

#endif //BUFFER_H