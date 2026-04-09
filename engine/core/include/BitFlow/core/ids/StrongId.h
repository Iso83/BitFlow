#pragma once

#include <cstdint>
#include <type_traits>

namespace BitFlow::Core::Ids {

template <typename Tag, typename ValueT = std::uint32_t> class StrongId {
    static_assert(std::is_integral_v<ValueT>, "StrongId requires an integral value type.");

  public:
    using ValueType = ValueT;

  private:
    ValueType m_value{};

  public:
    constexpr StrongId() = default;
    explicit constexpr StrongId(ValueType value) : m_value(value) {}

    [[nodiscard]] constexpr ValueType value() const {
        return m_value;
    }

    [[nodiscard]] constexpr bool isValid() const {
        return m_value != 0;
    }

    friend constexpr bool operator==(StrongId lhs, StrongId rhs) {
        return lhs.m_value == rhs.m_value;
    }

    friend constexpr bool operator!=(StrongId lhs, StrongId rhs) {
        return !(lhs == rhs);
    }
};

} // namespace BitFlow::Core::Ids
