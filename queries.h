#pragma once

#include <cstdint>

namespace DSVisualization {
    enum TreeQueryType { INSERT, ERASE, FIND };
    struct TreeQuery {
        TreeQueryType query_type;
        int32_t value;
    };
}// namespace DSVisualization