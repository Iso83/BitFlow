#pragma once

#include <BitFlow/core/rules/RuleKey.h>
#include <iostream>
#include <stdexcept>
#include <string>

namespace BitFlow::Core::Rules {
inline std::string ToString(RuleKey key) {
    return std::string(key.value);
}

[[noreturn]] inline void ThrowRuleError_Impl(const char* file, int line, const char* fn, const std::string& msg) {
    std::string full = "[RuleError] " + msg + "\n  at: " + fn + "\n  file: " + file + ":" + std::to_string(line);

#ifdef _DEBUG
    std::cerr << full << std::endl;
#endif

    throw std::runtime_error(full);
}

} // namespace BitFlow::Core::Rules

#define BF_RULE_ERROR(msg) BitFlow::Core::Rules::ThrowRuleError_Impl(__FILE__, __LINE__, __func__, (msg))
