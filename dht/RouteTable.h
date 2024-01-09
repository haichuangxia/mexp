#pragma once
#include "../trie/trie.h"
#include "Peer.h"
#include <map>
namespace dht
{
    class RouteTable
    {
    private:
        trie::trie<Contact> m_table;

    public:
        bool is_in_table(const Peer &peer);

        bool find(Peer &peer);

        bool insert(const Peer &peer);

        bool update(const Peer &peer);

        bool remove(Peer &peer);

        /**
         * @brief 从路由表中查找距离id最近的k个节点
         *
         * @param k 要查询的数量
         * @param body 查询到个结果
         * @return int 实际查询到的数量
         */
        int findKnearestPeer(int k, const NodeID &id, std::vector<Peer> &body) const;

        int findKnearestPeer(int k, KBucket &buckt) const;
    };
}