cmake_minimum_required(VERSION 3.23)
project(data_structure_visualization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(-Wall -Wextra -Wpedantic)
include(GoogleTest)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# set(CMAKE_CXX_FLAGS "-Os -DNDEBUG -Wall -Wextra")
add_executable(data_structure_visualization main.cpp application.cpp view.cpp model.cpp controller.cpp red_black_tree.cpp)
target_link_libraries(data_structure_visualization
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

add_executable(red_black_tree red_black_tree.cpp)

enable_testing()
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

add_executable(test_correctness red_black_tree.cpp tests/test_correctness.cpp tests/test_erase.cpp tests/test_insert.cpp tests/test_invariants.cpp)
target_link_libraries(test_correctness gtest gtest_main)

# set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
add_executable(test_performance red_black_tree.cpp tests/test_performance.cpp)
target_link_libraries(test_performance gtest gtest_main)