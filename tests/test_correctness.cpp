#include "../red_black_tree.h"
#include <gtest/gtest.h>
#include <random>
#include <set>

template<typename T>
std::vector<T> Values(const std::set<T>& s) {
    std::vector<T> values;
    for (auto& now : s) {
        values.push_back(now);
    }
    return values;
}

TEST(Correctness, RandomTestsInsertErase) {
    for (int test = 1; test <= 200; ++test) {
        std::mt19937 rnd(test);
        std::uniform_int_distribution<> uid(1, 50);
        std::uniform_int_distribution<> coin(1, 2);
        std::stringstream ss;
        RedBlackTree<int32_t> rb_tree;
        std::set<int32_t> s;
        for (int node = 1; node <= 1000; ++node) {
            if (coin(rnd) == 1) {
                int32_t value = uid(rnd);
                rb_tree.Insert(value);
                s.insert(value);
                ss << "insert " << value << "\n";
            } else {
                int32_t value = uid(rnd);
                rb_tree.Erase(value);
                s.erase(value);
                ss << "erase " << value << "\n";
            }
            ASSERT_TRUE(Values(s) == rb_tree.Values());
        }
    }
}