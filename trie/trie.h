#pragma once
#include "../type.h"
#include "Node.h"
#include <stdexcept>
namespace trie {

    template<typename T>
    struct trie {
        Node *root;
        void put(const byte *key,int key_len,T value);
        T get(byte *key,int key_len);
    private:
    };
}