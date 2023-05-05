cmake_minimum_required(VERSION 3.23)
project(data_structure_visualization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(data_structure_visualization main.cpp application.cpp view.cpp model.cpp controller.cpp)
target_link_libraries(data_structure_visualization
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

