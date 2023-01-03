#include "red_black_tree.h"
#include <iostream>
#include <gtest/gtest.h>

int main() {
    RedBlackTree<int> rb_tree;
    while (true) {
        std::string query_type;
        std::cin >> query_type;
        if (query_type == "insert") {
            int x;
            std::cin >> x;
            if (rb_tree.Insert(x)) {
                std::cout << "ok\n";
            } else {
                std::cout << "failed\n";
            }
            std::cout.flush();
            std::cout << rb_tree.Str();
        } else if (query_type == "break") {
            break;
        }
    }
    return 0;
}