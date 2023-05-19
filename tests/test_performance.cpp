#define NO_LOGGING
#include "../red_black_tree.h"
#include "../time_wrapper.h"

#include <iomanip>
#include <set>

#include <gtest/gtest.h>

namespace DSVisualization {

    TEST(Performance, Linear) {
        auto foo1 = [](int n) {
            RedBlackTree<int32_t> rb_tree;
            for (int i = 1; i <= n; ++i) {
                rb_tree.Insert(i);
            }
            for (int i = 1; i <= n; ++i) {
                rb_tree.Erase(i);
            }
        };
        for (int n : {10, 1'000, 200'000, 1'000'000}) {
            clock_t time = 0;
            TestTime(foo1, time).call(n);
            std::cout << "n = " << n << ": " << time / CLOCKS_PER_SEC << "." << time % CLOCKS_PER_SEC << "\n";
        }
        auto foo2 = [](int n) {
            std::set<int32_t> rb_tree;
            for (int i = 1; i <= n; ++i) {
                rb_tree.insert(i);
            }
            for (int i = 1; i <= n; ++i) {
                rb_tree.erase(i);
            }
        };
        for (int n : {10, 1'000, 200'000, 1'000'000}) {
            clock_t time = 0;
            TestTime(foo2, time).call(n);
            std::cout << "n = " << n << ": " << time / CLOCKS_PER_SEC << "." << time % CLOCKS_PER_SEC << "\n";
        }
    }
}// namespace DSVisualization