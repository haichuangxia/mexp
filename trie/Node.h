#pragma once
#include "../type.h"
namespace trie {
    enum class node_type:uint8_t {
        EXTENSION,
        BRANCH,
        LEAF,
        VALUE
    };
    struct Node {
        node_type type;
        virtual int prefix_len()=0;
        virtual int leaves_count()=0;
        virtual ~Node()=0;
    };

    template <typename T>
    struct Value: Node{
        T value;
        Value(){
            type=node_type::VALUE;
        };
        explicit Value(T _value):Value(),value(_value){};
        ~Value() override =default;
    };

    struct Branch: Node{
        Node* children[16];
        uint16_t leaves;
        int prefix_len() override{
          return 1;
        };
        int leaves_count() override{
            return this->leaves;
        };
        Branch(): leaves(0), children{} {};
        bool insert(Node *son, int loc);
        void replace(Node *son, int loc);
        void remove(uint8_t loc);
    };

    struct Extension: Node{
        uint8_t path_len;
        uint16_t leaves;
        byte * path;
        Branch* next;
        int prefix_len() override{
            return path_len;
        };
        int leaves_count() override{
            return this->leaves;
        }
        void insert(Branch* br);
        Extension(byte* key, int key_len);
        Extension(byte* key,int key_len,int start);
        ~Extension() override{
          delete[] path;
        };
    };

    template <typename T>
    struct Leaf: Node{
        uint8_t path_len;
        byte * path;
        T value;
        int prefix_len() override{
            return path_len;
        };
        int leaves_count() override{
            return 1;
        };
        Leaf()=delete;
        Leaf(const Leaf& another)=delete;
        Leaf(const Leaf&& another)=delete;
        Leaf& operator=(const Leaf& another)=delete;
        /**
         * 从缓冲区构造叶子节点
         * @param key 前缀缓冲区
         * @param key_len 新叶子节点前缀的长度
         * @param start 前缀的起始位置
         * @param value 值
         */
        Leaf(byte* key, int key_len, int start, T value);
        /**
         * 构造叶子节点
         * @param key 前缀的起始位置
         * @param key_len 前缀的长度
         * @param value 叶子节点的值
         */
        Leaf(byte* key, int key_len, T value);
        ~Leaf() override{
            delete[] path;
        };
    };

}
