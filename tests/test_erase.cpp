#include "../red_black_tree.h"

#include <gtest/gtest.h>

namespace DSVisualization {
    namespace {
        template<typename T>
        std::string RBTreeToString(const RedBlackTree<T>& rb_tree) {
            std::stringstream ss;
            ss << rb_tree;
            return ss.str();
        }
    }// namespace
    TEST(Erase, EmptyTree) {
        RedBlackTree<int> rb_tree;
        ASSERT_FALSE(rb_tree.Erase(1));
        std::string result = "Empty\n";
        EXPECT_EQ(RBTreeToString(rb_tree), result);
    }
}// namespace DSVisualization
