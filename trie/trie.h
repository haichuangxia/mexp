#pragma once
#include "../type.h"
#include "Node.h"
#include "Encode.h"
#include <iostream>
#include <stdexcept>
namespace trie
{

    template <typename T>
    struct trie
    {
        trie() = default;
        Node *root{nullptr};
        /**
         * @brief 放入原始字节编码的数据
         *
         * @param key 原始编码的key
         * @param key_len key的长度，要求所有节点key等长
         * @param value 插入的字段值
         */
        void put(const byte *key, int key_len, T value);

        /**
         * @brief 从trie中查询并取出元素
         *
         * @param key key缓冲区
         * @param key_len key的长度
         * @return T 返回一个副本
         */
        T get(byte *key, int key_len);

        /**
         * @brief 判断一个键值对是否在树中
         *
         * @param key
         * @param key_len
         * @return true
         * @return false
         */
        bool is_in_trie(byte *key, int key_len);

        /**
         * @brief Get the Ref object
         *
         * @param key 关键字指针
         * @param key_len 关键字长度
         * @return T& 值的引用
         */
        T &getRef(byte *key, int key_len);

    private:
        int nodeCount{0};
    };

    template <typename T>
    T trie<T>::get(byte *key, int key_len)
    {
        if (key == nullptr)
            throw std::invalid_argument("Pointer must not be nullptr");
        if (root == nullptr)
            return T();
        byte key_hex[2 * key_len];
        encode::hex_encode(key, key_len, key_hex, 2 * key_len);
        int start = 0;
        Node *current = this->root;
        while (current != nullptr)
        {
            switch (current->type)
            {
            case node_type::EXTENSION:
            {
                auto *ex = dynamic_cast<Extension *>(current);
		// TODO:segment default
                if (encode::compare(ex->path, 0, key_hex, start, ex->path_len) != -1)
                    return T();
                start += ex->path_len;
                current = ex->next;
                break;
            }
            case node_type::BRANCH:
            {
                auto *br = dynamic_cast<Branch *>(current);
                if (br->children[key_hex[start]] == nullptr)
                    return T();
                current = br->children[key_hex[++start]];
                break;
            }

            case node_type::LEAF:
            {
                auto *lf = dynamic_cast<Leaf<T> *>(current);
                if (encode::compare(lf->path, 0, key_hex, start, lf->path_len) == -1)
                    return lf->value;
                return T();
            }

            case node_type::VALUE:
            {
                return dynamic_cast<Value<T> *>(current)->value;
            }
            }
        }
        return T();
    }

    template <typename T>
    void trie<T>::put(const byte *key, int key_len, T value)
    {
        if (key == nullptr)
            throw std::invalid_argument("The key pointer is nullptr");
        byte key_hex[2 * key_len];
        encode::hex_encode(key, key_len, key_hex, 2 * key_len);
        // 根节点为空时
        if (root == nullptr)
        {
            root = new Leaf<T>(key_hex, 2 * key_len, 0, value);
            return;
        }
        int start = 0, mismatched, el_index = 0;
        Node *current = this->root, *father = nullptr;
        // trie中不止一个节点时
        while (current != nullptr)
        {
            switch (current->type)
            {
            case node_type::EXTENSION:
            {
                auto *ex = dynamic_cast<Extension *>(current);
                ++ex->leaves;
                mismatched = encode::compare(ex->path, 0, key_hex, start, ex->path_len);
                // 完全匹配
                if (mismatched == -1)
                {
                    start += ex->path_len;
                    father = current;
                    current = ex->next;
                }
                // 第一字节市配
                else if (mismatched == 0)
                {
                    // 创建后扩展节点
                    auto *pex = new Extension(ex->path + mismatched + 1, ex->path_len - mismatched - 1);
                    pex->insert(ex->next);
                    ex->next = nullptr;
                    // 创建叶子节点
                    auto *nl = new Leaf<T>(key_hex, (key_len << 1) - start - mismatched - 1, start + mismatched + 1,
                                           value);

                    // 将新插入节点和后扩展节点插入新分支节点
                    auto *br = new Branch();
                    br->insert(pex, ex->path[mismatched]);
                    br->insert(nl, key_hex[start + mismatched]);
                    // 使用新分支节点替换原有扩展节点
                    auto *fb = dynamic_cast<Branch *>(father);
                    fb->replace(br, el_index);
                    delete ex;
                    return;
                }
                //  最后一个字节失配
                else if (mismatched == ex->path_len - 1)
                {
                    // 将叶子节点和原扩展节点的后继插入到新分支节点中
                    auto *br = new Branch();
                    br->insert(ex->next, ex->path[mismatched]);
                    ex->next = nullptr;

                    auto *nl = new Leaf<T>(key_hex, (key_len << 1) - start - mismatched - 1, start + mismatched + 1,
                                           value);
                    br->insert(nl, key_hex[start + mismatched]);

                    // 将新分支节点插入到前扩展节点
                    auto *fex = new Extension(ex->path, mismatched);
                    fex->insert(br);
                    // 使用前扩展节点替换原节点
                    auto *fb = dynamic_cast<Branch *>(father);
                    fb->replace(fex, el_index);

                    delete ex;
                    return;
                }
                // 在中间失配
                else
                {
                    // 创建后扩展节点
                    auto *pex = new Extension(ex->path + mismatched + 1, ex->path_len - mismatched - 1);
                    pex->insert(ex->next);
                    ex->next = nullptr;
                    // 创建叶子节点
                    auto *nl = new Leaf<T>(key_hex, (key_len << 1) - start - mismatched - 1, start + mismatched + 1,
                                           value);

                    // 创建中分支节点
                    auto *br = new Branch();
                    br->insert(pex, ex->path[mismatched]);
                    br->insert(nl, key_hex[start + mismatched]);

                    // 创建前扩展节点
                    auto *fex = new Extension(ex->path, mismatched);
                    fex->insert(br);

                    // 替换原扩展节点
                    auto *fb = dynamic_cast<Branch *>(father);
                    fb->replace(fex, el_index);

                    // 回收原扩展节点
                    delete ex;
                    return;
                }
                break;
            }
            case node_type::BRANCH:
            {
                auto *br = dynamic_cast<Branch *>(current);
                el_index = key_hex[start++];
                ++br->leaves;
                if (br->children[el_index] == nullptr)
                {
                    br->children[el_index] = new Leaf<T>(key_hex, 2 * key_len, start, value);
                    return;
                }
                else
                {
                    father = current;
                    current = br->children[el_index];
                }
                break;
            }
            case node_type::LEAF:
            {
                auto *lf = dynamic_cast<Leaf<T> *>(current);
                mismatched = encode::compare(lf->path, 0, key_hex, start, lf->path_len);
                auto *nl = new Leaf<T>(key_hex, (key_len << 1) - start - mismatched - 1, start + mismatched + 1,
                                       value);
                auto *br = new Branch();
                // 如果完全不匹配
                if (mismatched == 0)
                {
                    auto *plf = new Leaf<T>(lf->path, lf->path_len - mismatched - 1, mismatched + 1, lf->value);
                    br->insert(plf, lf->path[mismatched]);
                    br->insert(nl, key_hex[start + mismatched]);
                    if (father == nullptr)
                    {
                        root = br;
                    }
                    else
                    {
                        auto *fb = dynamic_cast<Branch *>(father);
                        fb->replace(br, el_index);
                    }
                }
                // 如果只有最后一个字节不匹配
                else if (mismatched == lf->path_len - 1)
                {
                    // 创建值节点
                    auto *ori_value = new Value<T>(lf->value);
                    auto *new_value = new Value<T>(value);
                    // 插入到新分支节点
                    br->insert(ori_value, lf->path[mismatched]);
                    br->insert(new_value, key_hex[start + mismatched]);
                    // 创建扩展前缀节点
                    auto *fex = new Extension(lf->path, mismatched);
                    fex->insert(br);
                    // 替换原节点
                    if (father == nullptr)
                    {
                        root = fex;
                    }
                    else
                    {
                        dynamic_cast<Branch *>(father)->replace(fex, el_index);
                    }
                }
                // 中间节点不匹配
                else
                {
                    // 创建新叶子节点
                    auto *plf = new Leaf<T>(lf->path, lf->path_len - mismatched - 1, mismatched + 1, lf->value);
                    // 插入到分支节点
                    br->insert(plf, lf->path[mismatched]);
                    br->insert(nl, key_hex[start + mismatched]);
                    // 创建前扩展节点
                    auto *fex = new Extension(lf->path, mismatched);
                    fex->insert(br);
                    // 替换原节点
                    if (father == nullptr)
                        root = fex;
                    else
                        dynamic_cast<Branch *>(father)->replace(fex, el_index);
                }
                return;
            }
            case node_type::VALUE:
            {
                return;
            }
            }
        }
    }
}
