#pragma once

#include <cstdint>

namespace DSVisualization {
    enum class TreeQueryType { do_nothing, insert, erase, find };
    struct TreeQuery {
        TreeQueryType query_type = TreeQueryType::do_nothing;
        int value = 0;
    };
}// namespace DSVisualization