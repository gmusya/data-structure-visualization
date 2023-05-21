#include "../red_black_tree.h"

#include <random>
#include <set>

#include <gtest/gtest.h>

namespace DSVisualization {
    namespace {
        template<typename It>
        std::vector<typename It::value_type> Values(It begin, It end) {
            std::vector<typename It::value_type> result;
            for (auto it = begin; it != end; ++it) {
                result.push_back(*it);
            }
            return result;
        }
    }// namespace

    TEST(Correctness, RandomTestsInsertErase) {
        for (int test = 1; test <= 200; ++test) {
            std::mt19937 rnd(test);
            std::uniform_int_distribution<> uid(1, 50);
            std::uniform_int_distribution<> coin(1, 2);
            std::stringstream ss;
            DSVisualization::RedBlackTree<int32_t> rb_tree;
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
                ASSERT_TRUE(Values(s.begin(), s.end()) == Values(rb_tree.begin(), rb_tree.end()));
                ASSERT_TRUE(s.size() == rb_tree.Size());
                ASSERT_TRUE(s.empty() == rb_tree.Empty());
            }
        }
    }
}// namespace DSVisualization
