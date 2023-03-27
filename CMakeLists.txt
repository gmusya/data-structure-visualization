cmake_minimum_required(VERSION 3.23)
project(data_structures_visualization)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS "-fsanitize=address")

add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)
add_executable(tests tests/test_insert.cpp red_black_tree.cpp tests/test_invariants.cpp tests/test_correctness.cpp tests/test_erase.cpp)
target_link_libraries(tests gtest gtest_main)
add_executable(data_structures_visualization main.cpp red_black_tree.cpp)
