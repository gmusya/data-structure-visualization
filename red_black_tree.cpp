#define INVARIANTS_CHECK
#include "red_black_tree.h"
#include <gtest/gtest.h>
#include <iostream>

void PrintLines(std::ostream& os, int32_t depth) {
    if (depth > 0) {
        for (int32_t i = 1; i < depth; ++i) {
            os << "|   ";
        }
        os << "|---";
    }
}
