#include "../red_black_tree.h"
#include <gtest/gtest.h>

TEST(Insert, FirstInsert) {
    RedBlackTree<int> rb_tree;
    ASSERT_TRUE(rb_tree.Insert(1));
    std::string result = "(1, b)\n"
                         "|---(NIL, b)\n"
                         "|---(NIL, b)\n";
    EXPECT_EQ(rb_tree.Str(), result);
}

TEST(Insert, SecondInsert) {
    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(1));
        ASSERT_TRUE(rb_tree.Insert(3));
        std::string result = "(1, b)\n"
                             "|---(NIL, b)\n"
                             "|---(3, r)\n"
                             "|   |---(NIL, b)\n"
                             "|   |---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }

    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(5));
        ASSERT_TRUE(rb_tree.Insert(3));
        std::string result = "(5, b)\n"
                             "|---(3, r)\n"
                             "|   |---(NIL, b)\n"
                             "|   |---(NIL, b)\n"
                             "|---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }
}

TEST(Insert, OneRotation) {
    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(1));
        ASSERT_TRUE(rb_tree.Insert(3));
        ASSERT_TRUE(rb_tree.Insert(5));
        std::string result = "(3, b)\n"
                             "|---(1, r)\n"
                             "|   |---(NIL, b)\n"
                             "|   |---(NIL, b)\n"
                             "|---(5, r)\n"
                             "|   |---(NIL, b)\n"
                             "|   |---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }
    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(5));
        ASSERT_TRUE(rb_tree.Insert(3));
        ASSERT_TRUE(rb_tree.Insert(1));
        std::string result = "(3, b)\n"
                 "|---(1, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n"
                 "|---(5, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }
}

TEST(Insert, TwoRotations) {
    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(1));
        ASSERT_TRUE(rb_tree.Insert(3));
        ASSERT_TRUE(rb_tree.Insert(2));
        std::string result = "(2, b)\n"
                 "|---(1, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n"
                 "|---(3, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }
    {
        RedBlackTree<int> rb_tree;
        ASSERT_TRUE(rb_tree.Insert(3));
        ASSERT_TRUE(rb_tree.Insert(1));
        ASSERT_TRUE(rb_tree.Insert(2));
        std::string result = "(2, b)\n"
                 "|---(1, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n"
                 "|---(3, r)\n"
                 "|   |---(NIL, b)\n"
                 "|   |---(NIL, b)\n";
        EXPECT_EQ(rb_tree.Str(), result);
    }
}