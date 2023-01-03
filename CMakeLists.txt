cmake_minimum_required(VERSION 3.23)
project(data_structures_visualization)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)
add_executable(tests tests/tests.cpp red_black_tree.cpp tests/test_invariants.cpp)
target_link_libraries(tests gtest gtest_main)
add_executable(data_structures_visualization main.cpp red_black_tree.cpp)
