#define NO_LOGGING
#include "../red_black_tree.h"

#include <chrono>
#include <iomanip>
#include <set>

#include <gtest/gtest.h>

namespace DSVisualization {
    TEST(Performance, Linear) {
        for (int n : {10, 1000, 200000, 1000000}) {
            RedBlackTree<int32_t> rb_tree;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 1; i <= n; ++i) {
                rb_tree.Insert(i);
            }
            for (int i = 1; i <= n; ++i) {
                rb_tree.Erase(i);
            }
            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = finish - start;
            std::cout << "n = " << n << ": " << diff.count() << "\n";
        }
        for (int n : {10, 1000, 200000, 1000000}) {
            std::set<int32_t> rb_tree;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 1; i <= n; ++i) {
                rb_tree.insert(i);
            }
            for (int i = 1; i <= n; ++i) {
                rb_tree.erase(i);
            }
            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = finish - start;
            std::cout << "n = " << n << ": " << diff.count() << "\n";
        }
    }
}// namespace DSVisualization