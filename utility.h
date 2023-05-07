#pragma once

#include <chrono>
#include <ctime>

#define PRINT_WHERE_AM_I()                                                                         \
    do {                                                                                           \
        auto end = std::chrono::system_clock::now();                                               \
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);                          \
        std::string str(std::ctime(&end_time));                                                    \
        str = str.substr(0, str.size() - 1);                                                       \
        std::cerr << str << " " << __PRETTY_FUNCTION__ << " " << __FILE__ << ":" << __LINE__       \
                  << std::endl;                                                                    \
    } while (false)
