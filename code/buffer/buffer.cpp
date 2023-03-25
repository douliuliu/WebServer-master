/*
    Author       : liudou
    Date         : 2022-12-24
*/
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 缓冲区中还需要读的字节数量
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
// 获取缓冲区中还可以写多少字节数
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

// 缓冲区中已经读完字节数量，这部分内存可以再次利用
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

// 读的位置的地址
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

// 将读位置加上http请求报文一行的长度
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// 保证读位置小于下一行位置，然后将读位置加上http请求报文一行的长度，表示已经读了一行
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

// 将缓冲区清空全置为0，读写位置变为开头
void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

// 以缓冲区所有还需要读的字符构造一个string字符串，然后清空缓冲区并返回该字符串
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

// 写的位置的地址
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

// 缓冲区可以开始写的位置的地址
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

// 已经写了len长度字节，将writePos_（写的位置）加len
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

// 从str地址头开始，将str所代表的字符串写入缓冲区
void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

// 从data转为字符类型指针的地址头开始，将len长度的字节写入缓冲区
void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

// 从str地址头开始，将len长度的字节写入缓冲区,保证有可以用的空间，且从writePos_即写的位置开始写
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

// 将参数里缓冲区的所有可读字节写入到this指针所指对象的缓冲区中
void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

// 读取客户端的数据，将文件描述符的内核缓冲区数据读到我们的读缓冲区中
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];                               // 临时的数组，保证能够把所有的数据都读出来
    struct iovec iov[2];
    const size_t writable = WritableBytes();        // 获取缓冲区中还可以写多少字节数
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        HasWritten(len);
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

// 缓冲区开始地址
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

// 缓冲区开始地址
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}