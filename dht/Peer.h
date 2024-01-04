#pragma once

#include "../type.h"
#include "../net/Address.h"
#include <string>
#include <cstring>//memcmp函数比较字节序

namespace dht {
    struct NodeID{
        byte m_id[ID_LEN]{0};
        void distance(const NodeID&other){
            for (int i = 0; i < sizeof(m_id); ++i)
                m_id[i]^=other.m_id[i];
        };
        bool operator<(const NodeID&other) const{
            return std::memcmp(this->m_id,other.m_id,ID_LEN)<0;
        };
        bool operator==(const NodeID&other) const{
            return  std::memcmp(this->m_id,other.m_id,ID_LEN)==0;
        }
        bool operator>(const NodeID&other) const{
            return std::memcmp(this->m_id,other.m_id,ID_LEN)>0;
        }
    };
    struct NodePub{
        byte m_pub[PUB_LEN]{0};
    };
    struct Contact{
       NodePub m_pub;
       net::Address m_address{};
    };
    struct Peer {
        NodeID m_id;
        Contact m_contact;
        bool operator<(Peer other) const{
            return m_id<other->m_id;
        };

        bool operator>(const Peer*other) const{
            return m_id>other->m_id;
        };
    };

} // dht
