#pragma once

#include "Peer.h"
#include <set>
#include <vector>
#include <atomic>
#include "RouteTable.h"
#include "../net/UDPSocketWrapper.h"
namespace dht
{
    class KBucket
    {
    private:
        NodeID base;
        int size = 0;
        int capacity{ROUTING_TABLE_BUCKET_SIZE};

    public:
        std::mutex peerMapMtx;                    // 确保互斥访问bucket
        std::atomic<int> isWorking{0};            // 正常工作的线程数
        std::atomic<bool> shouldTerminate{false}; // 是否已经找到了目标节点
        std::condition_variable peerMaxCv;
        std::map<NodeID, Peer> m_bucket;

    public:
        KBucket() = default;

        KBucket(const NodeID &bench) : base(bench){};

        /**
         * @brief 向k桶中插入节点
         *
         * @param other 从其他节点处查询到的新节点
         * @return int -1：已经存在该节点，0：桶已满，且该节点并非更小，1：成功插入新节点
         */
        int insert(const Peer &other);

        inline auto min()
        {
            return m_bucket.begin();
        }

        inline void remove(std::map<NodeID, Peer>::iterator it)
        {
            std::lock_guard<std::mutex> peerMapMtxLock(peerMapMtx);
            if (it != m_bucket.end())
                m_bucket.erase(it);
        }
    };

    // Kademlia协议
    class Kademlia
    {
    private:
        net::UdpTransport &transport{net::UdpTransport::getUDPSocket()};
        RouteTable routeTable;
        Peer self;

    public:
        std::mutex routeTableLock;

    public:
        /**
         * @brief 查询一个节点是否在线
         *
         * @param peer 节点
         * @return int 1：在线，0：不在线，<0:出现了异常，如超时未响应，连接建立失败
         */
        int ping(const Peer &peer);

        /**
         * @brief Kademlia接收到其他用户发送的ping消息时的回调函数
         *
         * @param msg net::Message对象，主要包括请求方的地址和包含消息的缓冲区
         * @return int >0:发送pong消息成功
         */
        int onPing(net::Message &msg);

        bool connect(Peer &peer);

        /**
         * @brief 执行一次查询，如果出现了更近的节点则更新路由表
         *
         * @param peer 要询问的对象
         * @param key  要获取的节点信息
         * @return int <0:出现了异常，0：对方响应了，但是没有更新路由表，1:更新了路由表
         */
        int find_node(Peer &peer, const NodeID &key, KBucket &bucket);

        int findThead(Peer &peer, KBucket &bucket);

        /**
         * @brief 从路由表中查询对象的信息
         *
         * @param [in/out] peer 根据对象的key获取其地址
         * @return int <0:出现了异常，0：没有获取到，1：成功获取到
         */
        int find(Peer &peer);

        /**
         * @brief 收到请求方发出的FindNode请求时的回调函数
         *
         * @param msg 包含消息的缓冲区
         */
        void onFindNodeReqeust(net::Message &msg);

        void onReceiveRequest();

        void init();
    };
}
