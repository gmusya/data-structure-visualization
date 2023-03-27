#include "../red_black_tree.h"
#include <gtest/gtest.h>

TEST(Erase, EmptyTree) {
    RedBlackTree<int> rb_tree;
    ASSERT_FALSE(rb_tree.Erase(1));
    std::string result = "Empty\n";
    EXPECT_EQ(rb_tree.Str(), result);
}
