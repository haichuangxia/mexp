#include <gtest/gtest.h>
#include "trie.h"
#include <cstdint>
#include <random>
/**
 * @brief 生成一个指定长度的随机数
 *
 * @param keyLen 要生成的随机数长度
 * @param buffer 地址
 */
void generateRandomKey(uint8_t *buffer, int keyLen)
{
    std::random_device rd;

    // 创建一个随机数引擎
    std::default_random_engine gen(rd());

    // 创建一个分布器，指定生成均匀分布的字节
    std::uniform_int_distribution<uint8_t> distribution(0, 255);

    // 生成随机数
    for (int i = 0; i < keyLen; ++i)
    {
        *(buffer + i) = distribution(gen);
    }
};

TEST(trie, insert)
{
    trie::trie<int> inttrie;
    auto buffer = new uint8_t[32];
    for (int i = 0; i < 2; ++i)
    {
        generateRandomKey(buffer, 32);
        inttrie.put(buffer, 32, i);
        EXPECT_EQ(inttrie.get(buffer, 32), i);
    }
};

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};