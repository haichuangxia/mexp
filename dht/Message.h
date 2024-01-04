#pragma once


namespace dht {

    enum class MessageType{
        KPing,
        KFindNode,
        KConnect
    };
    class Message {
    private:
        MessageType m_type;
    };

}

