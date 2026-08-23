#pragma once

#include <string>

inline bool Contains(const std::string& s, const char* text) {
    return s.find(text) != std::string::npos;
}

inline bool Contains(const std::string& s, char ch) {
    return s.find(ch) != std::string::npos;
}