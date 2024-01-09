#pragma once
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "Address.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../comm/Buffer.h"
#include "Message.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace net
{
    // 主动发起的连接：主要用于ping和FindNode这种需要对方返回消息的调用
    struct Connection
    {
        uint64_t connId;
        std::mutex mtx;
        std::condition_variable cv;
        comm::Buffer buf;
        Connection(uint64_t id) : connId(id){};
    };

    class UdpTransport
    {
    private:
        int socketfd;
        Address myself;       // 本地监听地址
        Address peer;         // 消息发送方地址
        std::thread listener; // 监听socket进程
        comm::Buffer recvBuf; // socket的接收缓存，用于recvfrom存储一个包

    public:
        std::condition_variable cv;                           // 条件变量，唤醒等待的线程
        std::mutex connMutex;                                 // connections锁，防止多个线程同时建立多个连接
        std::unordered_map<uint64_t, Connection> connections; // 每一个对应一个连接

        net::Message msg;
        std::mutex recvBufMtx;
        std::condition_variable receiveRequestCv;

    public:
        /**
         * @brief 连接到一个地址
         *
         * @param peer 要连接的对象
         * @return uint64_t 0：失败，>0:相当于文件描述符，connectionID
         */
        uint64_t connect(Address peer);

        /**
         * @brief 关闭一个已有连接
         *
         * @param connID 要关闭的连接的ID
         */
        void close(uint64_t connID);

        // 通过局部静态变量获取单例
        static UdpTransport &getUDPSocket();

        ~UdpTransport();

        UdpTransport(const UdpTransport &) = delete;

        UdpTransport &operator=(const UdpTransport &) = delete;

        UdpTransport(UdpTransport &&another) = delete;

        UdpTransport &operator=(UdpTransport &&another) = delete;

        /**
         * @brief 通过UDP socket向对方发送数据
         *
         * @param
         * @param buffer
         * @return int
         */
        int sendto(const Address &peer, const comm::Buffer &buffer);

        /**
         * @brief 初始化
         *
         * @param me 要监听的地址
         */
        void initSocket(Address me);

    private:
        UdpTransport();

        /**
         * @brief 一个监听线程，用于监听socket
         *
         * @param sockfd 要监听的文件描述符
         * @param addr 要监听的地址
         */
        void listen(int sockfd, Address addr);
    };

    UdpTransport::UdpTransport() : socketfd(-1), recvBuf(RECV_BUF_SIZE){};

    UdpTransport::~UdpTransport()
    {
        if (socketfd != -1)
            close(socketfd);
    }

    uint64_t UdpTransport::connect(Address peer)
    {
        if (connections.find(peer) != connections.end())
        {
            connections.emplace(peer, Connection(peer));
            return static_cast<uint64_t>(peer);
        }
        return 0;
    };

    void UdpTransport::close(uint64_t connID)
    {
        // TODO:如果还有其他线程可能关闭该连接，如后面设置了超时计时器，则这里可能还是要用锁
        if (connections.find(connID) != connections.end())
            connections.erase(connID);
    };

    UdpTransport &UdpTransport::getUDPSocket()
    {
        static UdpTransport udpTransport;
        return udpTransport;
    };
}
