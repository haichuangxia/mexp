#pragma once

#include "Peer.h"
#include <set>
#include <vector>

namespace dht {
    // K桶，考虑使用优先队列，同时需要考虑去重
    class KBucket {
    private:
        std::set<Peer> m_bucket;
        int capacity;
    public:
        // 未满则直接直接插入，已满则看是否小于最大值，如果是则先删除最大值，然后插入
        void insert(const Peer &other){
            //已存在则直接返回
            if (m_bucket.find(other) != m_bucket.end()) return;
            if(m_bucket.size()<capacity) m_bucket.insert(other);//未满则直接插入
            //已满且更小则替换
            else if(other<*m_bucket.rbegin()){
                m_bucket.erase(*m_bucket.rbegin());
                m_bucket.insert(other);
            } else return;
        };

        inline auto min() -> std::set<Peer>::iterator {
            return m_bucket.begin();
        }

        inline void remove(std::set<Peer>::iterator it) {
            if (it != m_bucket.end())
                m_bucket.erase(it);
        }
    };

    class Kademlia {
    private:

    public:
        bool ping(const Peer &peer);

        bool connect(Peer &peer);

        bool find_node(Peer &peer);
    };
}


