cmake_minimum_required(VERSION 3.22)
project(data_structure_visualization)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ASAN OFF)
set(TSAN OFF)
set(LOGGING ON)
set(EXCEPTION_HANDLING OFF)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(LOGGING OFF)
endif()

if (NOT LOGGING)
    add_compile_definitions(NO_LOGGING)
endif()

if (ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif ()

if (TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif ()

if (NOT EXCEPTION_HANDLING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif ()

add_executable(data_structure_visualization main.cpp application.cpp view.cpp controller.cpp main_window.cpp)
if (MSVC)
    target_compile_options(data_structure_visualization PRIVATE /W4 /WX)
else ()
    target_compile_options(data_structure_visualization PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

target_link_libraries(data_structure_visualization
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

include(GoogleTest)

enable_testing()
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

add_executable(test_correctness tests/test_insert.cpp tests/test_erase.cpp tests/test_correctness.cpp)
add_executable(test_invariants tests/test_invariants.cpp)
add_executable(test_performance tests/test_performance.cpp)

target_link_libraries(test_correctness gtest gtest_main)
target_link_libraries(test_invariants gtest gtest_main)
target_link_libraries(test_performance gtest gtest_main)
