#pragma once
#include "../type.h"
#include <cstring>
namespace trie
{
    enum class node_type : uint8_t
    {
        EXTENSION,
        BRANCH,
        LEAF,
        VALUE
    };
    struct Node
    {
        node_type type;
        Node() = default;
        Node(node_type _type) : type(_type){};
        virtual int prefix_len() = 0;
        virtual int leaves_count() = 0;
        virtual ~Node() = default;
    };

    template <typename T>
    struct Value : Node
    {
        T value;

        Value()
        {
            type = node_type::VALUE;
        };
        int prefix_len() override
        {
            return 0;
        };

        int leaves_count() override
        {
            return 1;
        };

        explicit Value(T _value) : value(_value)
        {
            type = node_type::VALUE;
        };
        ~Value() override = default;
    };

    struct Branch : Node
    {
        uint16_t leaves{0};
        Node *children[16]{nullptr};
        int prefix_len() override
        {
            return 1;
        };
        int leaves_count() override
        {
            return this->leaves;
        };
        Branch()
        {
            type = node_type::BRANCH;
        };
        bool insert(Node *son, int loc);
        void replace(Node *son, int loc);
        void remove(uint8_t loc);
    };

    struct Extension : Node
    {
        uint8_t path_len{0};
        uint16_t leaves{0};
        byte *path{nullptr};
        Branch *next{nullptr};
        int prefix_len() override
        {
            return path_len;
        };
        int leaves_count() override
        {
            return this->leaves;
        }
        void insert(Branch *br);
        Extension(byte *key, int key_len);
        Extension(byte *key, int key_len, int start);
        ~Extension() override
        {
            delete[] path;
        };
    };

    template <typename T>
    struct Leaf : Node
    {
        uint8_t path_len{0};
        byte *path{nullptr};
        T value;
        int prefix_len() override
        {
            return path_len;
        };
        int leaves_count() override
        {
            return 1;
        };
        Leaf() = default;
        Leaf(const Leaf &another) = delete;
        Leaf(const Leaf &&another) = delete;
        Leaf &operator=(const Leaf &another) = delete;
        /**
         * 从缓冲区构造叶子节点
         * @param key 前缀缓冲区
         * @param key_len 新叶子节点前缀的长度
         * @param start 前缀的起始位置
         * @param value 值
         */
        Leaf(byte *key, int key_len, int start, T value);
        /**
         * 构造叶子节点
         * @param key 前缀的起始位置
         * @param key_len 前缀的长度
         * @param value 叶子节点的值
         */
        Leaf(byte *key, int key_len, T value);
        ~Leaf() override
        {
            delete[] path;
        };
    };

    template <typename T>
    Leaf<T>::Leaf(byte *key, int key_len, T value) : path_len(key_len)
    {
        type = node_type::LEAF;
        path = new byte[key_len];
        memcpy(path, key, key_len);
    }

    template <typename T>
    Leaf<T>::Leaf(byte *key, int key_len, int start, T value) : path_len(key_len), value(value)
    {
        type = node_type::LEAF;
        path = new byte[key_len];
        memcpy(path, key + start, key_len);
    };
}
