#pragma once

#include <iostream>
#include <stdexcept>

#define BF_ASSERT(expr)                                                                                                \
    do {                                                                                                               \
        if (!(expr))                                                                                                   \
            throw std::runtime_error(std::string("Assertion failed: ") + #expr + "\nFile: " + __FILE__ +               \
                                     "\nLine: " + std::to_string(__LINE__));                                           \
    } while (0)
