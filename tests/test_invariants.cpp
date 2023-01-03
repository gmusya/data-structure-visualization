#define INVARIANTS_CHECK
#include "../red_black_tree.h"
#include <gtest/gtest.h>
#include <numeric>
#include <random>

TEST(Invariants, SmallTestsInsert) {
    for (int size = 1; size <= 9; ++size) {
        std::vector<int> p(size);
        std::iota(p.begin(), p.end(), 1);
        do {
            RedBlackTree<int> rb_tree;
            for (int x : p) {
                ASSERT_TRUE(rb_tree.Insert(x));
                ASSERT_TRUE(rb_tree.CheckInvariants());
            }
        } while (std::next_permutation(p.begin(), p.end()));
    }
}

TEST(Invariants, RandomTestsInsert) {
    for (int test = 1; test <= 1000; ++test) {
        std::mt19937 rnd(test);
        std::uniform_int_distribution<> uid(1, 1000);
        RedBlackTree<int32_t> rb_tree;
        for (int node = 1; node <= 200; ++node) {
            int32_t value = uid(rnd);
            rb_tree.Insert(value);
            ASSERT_TRUE(rb_tree.CheckInvariants());
        }
    }
}