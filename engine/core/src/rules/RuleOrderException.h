#pragma once

#include <stdexcept>

namespace BitFlow::Core::Rules {

class RuleOrderException : public std::logic_error {
  public:
    explicit RuleOrderException(const char* msg) : std::logic_error(msg) {}
};

} // namespace BitFlow::Core::Rules