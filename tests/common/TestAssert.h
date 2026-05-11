#pragma once

#include <iostream>
#include <stdexcept>

#define BF_TEST(expr)                                                                                                  \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            std::cerr << "Test failed: " << #expr << " | file: " << __FILE__ << " | line: " << __LINE__ << std::endl;  \
            return -1;                                                                                                 \
        }                                                                                                              \
    } while (0)

#define BF_RUN_TEST(func, ...)                                                                                         \
    do {                                                                                                               \
        if (func(__VA_ARGS__) != 0)                                                                                    \
            return -1;                                                                                                 \
    } while (0)

#define BF_ASSERT(expr)                                                                                                \
    do {                                                                                                               \
        if (!(expr))                                                                                                   \
            throw std::runtime_error(std::string("Assertion failed: ") + #expr + "\nFile: " + __FILE__ +               \
                                     "\nLine: " + std::to_string(__LINE__));                                           \
    } while (0)