#pragma once
#include "../comm/ISerilize.h"
namespace dht
{

    enum class MessageType : uint8_t
    {
        KPing = 0x01,
        KFindNodeRequest = 0x02,
        KPong = 0x11,
        KFindNodeReponse = 0x12,
        KConnect
    };

    struct Message : comm::ISerilize
    {
        MessageType m_type;

        Message() = default;

        Message(MessageType type) : m_type(type){};

        int size() const override
        {
            return sizeof(m_type);
        };

        int serialize(byte *buf, int buf_size) const override
        {
            memcpy(buf, &m_type, sizeof(m_type));
            return sizeof(m_type);
        };

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < sizeof(m_type))
                return -1;
            memcpy(&m_type, buf, sizeof(m_type));
            return sizeof(m_type);
        }
    };

    struct PingMsg : Message
    {
        NodeID remote;

        PingMsg() = default;

        PingMsg(const NodeID &id) : remote(id)
        {
            m_type = MessageType::KPing;
        };

        int size() const override
        {
            return Message::size() + remote.size();
        };

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;

            int offset = Message::serialize(buf, capacity);
            return offset + remote.serialize(buf + offset, capacity - offset);
        };

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < size())
                return -1;
            int offset = Message::deserialize(buf, capacity);
            offset += remote.deserialize(buf + offset, capacity - offset);

            return offset;
        }
    };

    struct PongMsg : Message
    {
        NodeID local;

        PongMsg() = default;

        PongMsg(const NodeID &other)
        {
            memcpy(local.m_id, other.m_id, ID_LEN);
        };

        int size() const override
        {
            return Message::size() + local.size();
        }

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;

            int offset = Message::serialize(buf, capacity);
            return offset + local.serialize(buf + offset, capacity - offset);
        }

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < size())
                return -1;
            int offset = Message::deserialize(buf, capacity);
            offset += local.deserialize(buf + offset, capacity - offset);
            return offset;
        }
    };

    struct FindNodeRequestMsg : Message
    {
        uint8_t k;

        NodeID key;

        FindNodeRequestMsg() = default;

        FindNodeRequestMsg(const NodeID &peerId) : k(ROUTING_TABLE_BUCKET_SIZE), key(peerId){};

        int size() const override
        {
            return Message::size() + sizeof(k) + key.size();
        };

        int serialize(byte *buf, int capacity) const override
        {
            if (capacity < size())
                return -1;

            int offset = Message::serialize(buf, capacity);
            memcpy(buf + offset, &k, sizeof(k));
            offset += sizeof(k);
            offset += key.serialize(buf + offset, capacity - offset);
            return offset;
        };

        int deserialize(const byte *buf, int capacity) override
        {
            if (capacity < size())
                return -1;

            int offset = Message::deserialize(buf, capacity);
            memcpy(&k, buf + offset, sizeof(k));
            offset += sizeof(k);
            offset += key.deserialize(buf + offset, capacity - offset);
            return offset;
        }
    };

    struct FindNodeResponseMsg : Message
    {
        uint8_t ksize;
        std::vector<Peer> knear;

        FindNodeResponseMsg() : Message(MessageType::KFindNodeReponse){};

        int size() const override;

        int serialize(byte *buf, int capacity) const override;

        int deserialize(const byte *buf, int capacity) override;
    };
}
