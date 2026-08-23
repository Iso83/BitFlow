#pragma once

#include <string_view>

namespace BitFlow::Engine::Core::Rules {

struct RuleKey {
    std::string_view value{};

    constexpr RuleKey(const char* v) : value(v) {}

    constexpr bool operator==(const RuleKey& other) const noexcept {
        return std::string_view(value) == std::string_view(other.value);
    }
};

} // namespace BitFlow::Core::Rules

namespace std {

template <> struct hash<BitFlow::Engine::Core::Rules::RuleKey> {
    size_t operator()(const BitFlow::Engine::Core::Rules::RuleKey& k) const noexcept {
        return std::hash<std::string_view>{}(k.value);
    }
};

} // namespace std
