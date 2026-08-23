#pragma once

#include <stdexcept>
#include <string>

namespace BitFlow::Engine::Core {

class CoreException : public std::runtime_error {
  public:
    explicit CoreException(const std::string& message) : std::runtime_error(message) {}
};

class CoreInvalidArgument : public std::invalid_argument {
  public:
    explicit CoreInvalidArgument(const std::string& message) : std::invalid_argument(message) {}
};

} // namespace BitFlow::Core

#define BF_CORE_THROW(msg) throw ::BitFlow::Engine::Core::CoreException(msg)

#define BF_CORE_THROW_INVALID_ARGS(msg) throw ::BitFlow::Engine::Core::CoreInvalidArgument(msg)
