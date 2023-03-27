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

TEST(Invariants, SmallTestsInsertErase) {
    for (int size = 1; size <= 8; ++size) {
        for (int y = 1; y <= size; ++y) {
            std::vector<int> p(size);
            std::iota(p.begin(), p.end(), 1);
            do {
                RedBlackTree<int> rb_tree;
                for (int x : p) {
                    ASSERT_TRUE(rb_tree.Insert(x));
                    ASSERT_TRUE(rb_tree.CheckInvariants());
                }
                ASSERT_TRUE(rb_tree.Erase(y));
                ASSERT_TRUE(rb_tree.CheckInvariants());
            } while (std::next_permutation(p.begin(), p.end()));
        }
    }
}

TEST(Invariants, RandomTestsInsertErase) {
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
            ASSERT_TRUE(rb_tree.CheckInvariants());
            ASSERT_TRUE(s.size() == rb_tree.Size());
            ASSERT_TRUE(s.empty() == rb_tree.Empty());
        }
    }
}
