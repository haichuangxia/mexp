#include "UDPSocketWrapper.h"
#include <cstring>
namespace net
{

    void UdpTransport::listen(int sockfd, Address aadr)
    {
        int fd = sockfd;

        sockaddr_in local, from;
        std::memset(&local, 0, sizeof(local));
        std::memset(&from, 0, sizeof(from));

        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY;
        local.sin_port = aadr.m_port;
        // 绑定监听地址
        if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0)
            throw std::runtime_error("Can`t bind socket to listen address");
        // 接收缓冲区
        socklen_t from_len = sizeof(from);

        // 循环监听获取数据
        while (recvBuf.m_size = recvfrom(fd, recvBuf.m_buffer, recvBuf.m_capacity, 0, (sockaddr *)&from, &from_len))
        {
            Address peer(from);
            // 说明是对方回传的数据，数据类型是response,此时唤醒连接队列
            if (connections.find(peer) != connections.end())
            {
                memcpy(connections[peer].buf.m_buffer, recvBuf.m_buffer, recvBuf.m_size);
                connections[peer].buf.m_size += recvBuf.m_size;
                cv.notify_all(); // 唤醒等待的线程
            }
            // 说明是对方请求的数据，默认对方的请求都是小数据包
            else
            {
                receiveRequestCv.notify_all();
            }
        }
    };
    void UdpTransport::initSocket(Address me)
    {

        if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            throw std::runtime_error("Failed to create UDP socket");
    }
}