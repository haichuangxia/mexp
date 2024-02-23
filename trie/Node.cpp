#include "Node.h"
#include <cstring>
#include <stdexcept>
#include <iostream>
namespace trie
{
    Extension::Extension(byte *key, int key_len) : path_len(key_len), next(nullptr)
    {
        path = new byte[key_len];
        memcpy(path, key, key_len);
        type = node_type::EXTENSION;
    };

    Extension::Extension(byte *key, int key_len, int start) : path_len(key_len), next(nullptr)
    {
        path = new byte[key_len];
        memcpy(path, key + start, key_len);
        type = node_type::EXTENSION;
    };

    void Extension::insert(trie::Branch *br)
    {
        this->next = br;
        leaves = br->leaves_count();
    }

    bool Branch::insert(Node *son, int loc)
    {
        if (loc > 15 || loc < 0)
        {
            throw std::out_of_range("The insert location of new Node in Branch Node is out of range");
        }
        if (son == nullptr)
            throw std::invalid_argument("the new Node can`t be null");
        if (children[loc] != nullptr)
            throw std::invalid_argument("The location wanted to insert is not null");
        switch (son->type)
        {
        case node_type::BRANCH:
        {
            son = dynamic_cast<Branch *>(son);
            this->leaves += son->leaves_count();
            this->children[loc] = son;
            break;
        }
        case node_type::LEAF:
        {
            this->leaves += 1;
            this->children[loc] = son;
            break;
        }
        case node_type::EXTENSION:
        {
            son = dynamic_cast<Extension *>(son);
            this->leaves += son->leaves_count();
            this->children[loc] = son;
            break;
        }
        case node_type::VALUE:
        {
            ++this->leaves;
            this->children[loc] = son;
            break;
        }
        }
        this->children[loc] = son;
        return true;
    };

    void Branch::replace(Node *son, int loc)
    {
        if (loc > 15 || loc < 0)
        {
            throw std::out_of_range("The insert location of new Node in Branch Node is out of range");
        }
        if (son == nullptr)
            throw std::invalid_argument("the new Node can`t be null");
        if (children[loc] == nullptr)
        {
            children[loc] = son;
            leaves += son->leaves_count();
            return;
        }
        this->leaves += children[loc] == nullptr ? son->leaves_count() : son->leaves_count() - children[loc]->leaves_count();
        children[loc] = son;
    }

    void Branch::remove(uint8_t loc)
    {
        if (loc > 15)
            throw std::out_of_range("The index of Node wanted to be removed is out of range 0-15");
        if (children[loc] == nullptr)
            return;
        this->leaves -= children[loc]->leaves_count();
        delete children[loc];
        children[loc] = nullptr;
    }
}
