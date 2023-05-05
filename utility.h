#define PRINT_WHERE_AM_I()                                                                         \
    do                                                                                             \
        std::cerr << __PRETTY_FUNCTION__ << " " << __FILE__ << ":" << __LINE__ << std::endl;       \
    while (false)

