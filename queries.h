#pragma once

#include <cstdint>

namespace DSVisualization {
    enum TreeQueryType { DO_NOTHING, INSERT, ERASE, FIND,  };
    struct TreeQuery {
        TreeQueryType query_type = DO_NOTHING;
        int value = 0;
    };
}// namespace DSVisualization