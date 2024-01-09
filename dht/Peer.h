#pragma once

#include <string>
#include <cstring>
#include <algorithm>
#include "../type.h"
#include "../net/Address.h"
#include "../comm/ISerilize.h"

namespace dht
{
    struct NodeID : comm::ISerilize
    {
        byte m_id[ID_LEN]{0};

        bool isZero() const
        {
            return std::all_of(m_id, m_id + ID_LEN, [](byte b)
                               { return b == 0; });
        };

        void distance(const NodeID &other)
        {
            for (int i = 0; i < sizeof(m_id); ++i)
                m_id[i] ^= other.m_id[i];
        };

        NodeID countDistance(const NodeID &other) const;

        bool operator<(const NodeID &other) const
        {
            return std::memcmp(this->m_id, other.m_id, ID_LEN) < 0;
        };
        bool operator==(const NodeID &other) const
        {
            return std::memcmp(this->m_id, other.m_id, ID_LEN) == 0;
        }
        bool operator>(const NodeID &other) const
        {
            return std::memcmp(this->m_id, other.m_id, ID_LEN) > 0;
        }

        int size() const override
        {
            return sizeof(m_id);
        }

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;
            memcpy(buf, m_id, sizeof(m_id));
            return size();
        };

        int deserialize(const byte *buf, int size) override
        {
            if (size < sizeof(m_id))
                return -1;
            memcpy(m_id, buf, sizeof(m_id));
            return sizeof(m_id);
        }
    };
    struct NodePub : comm::ISerilize
    {
        byte m_pub[PUB_LEN]{0};

        bool isZero() const
        {
            return std::all_of(m_pub, m_pub + PUB_LEN, [](byte b)
                               { return b == 0; });
        };
        int size() const override
        {
            return sizeof(m_pub);
        }

        int serialize(byte *buf, int buf_size) const override
        {
            if (buf_size < size())
                return -1;
            memcpy(buf, m_pub, sizeof(m_pub));
            return size();
        };

        int NodePub::deserialize(const byte *buf, int size) override
        {
            if (size < sizeof(m_pub))
                return -1;
            memcpy(m_pub, buf, sizeof(m_pub));
            return sizeof(m_pub);
        }
    };
    struct Contact : comm::ISerilize
    {
        NodePub m_pub;
        net::Address m_address{};

        int size() const override
        {
            return m_pub.size() + m_address.size();
        }

        bool isZero() const
        {
            return m_pub.isZero() && m_address.isZero();
        };

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;
            int start = m_pub.serialize(buf, capacity);
            start += m_address.serialize(buf + start, capacity - start);
            return start;
        };

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < size())
                return -1;
            int offset = m_pub.deserialize(buf, capacity);
            offset += m_address.deserialize(buf + offset, capacity - offset);
            return offset;
        };
    };
    struct Peer : comm::ISerilize
    {
        NodeID m_id;
        Contact m_contact;

        inline int size() const override
        {
            return m_id.size() + m_contact.size();
        };

        bool isZero() const
        {
            return m_id.isZero() && m_contact.isZero();
        };

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;
            int start = m_id.serialize(buf, capacity);
            start += m_contact.serialize(buf + start, capacity - start);
            return start;
        }

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < size())
                return -1;
            int start = m_id.deserialize(buf, capacity);
            start += m_contact.deserialize(buf + start, capacity - start);
            return start;
        };

        bool operator<(const Peer &other) const
        {
            return m_id < other.m_id;
        };

        bool operator>(const Peer &other) const
        {
            return m_id > other.m_id;
        };
    };

}
