#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::BitVector {

class bf_uint {
  private:
    uint32_t m_bw = 0;
    std::vector<uint64_t> m_words;

  public:
    enum class StringBase : uint8_t { Binary = 2, Decimal = 10, Hex = 16 };

  public:
    explicit bf_uint(uint32_t bitWidth);
    explicit bf_uint(uint64_t value, uint32_t bitWidth);

    uint32_t BitWidth() const noexcept {
        return m_bw;
    }

    bool IsZero() const noexcept {
        for (uint64_t w : m_words) {
            if (w != 0ULL)
                return false;
        }
        return true;
    }

    uint64_t ToUint64() const noexcept {
        return m_words.empty() ? 0ULL : m_words[0];
    }
    uint32_t ToUint32() const noexcept {
        return static_cast<uint32_t>(ToUint64() & 0xffffffffULL);
    }

    // string conversion
    std::string ToString(StringBase base = StringBase::Decimal) const;
    std::string ToBinaryString() const;
    std::string ToDecimalString() const;
    std::string ToHexString(bool upperCase = false) const;

    // comparison
    bool operator==(const bf_uint& rhs) const noexcept;
    bool operator!=(const bf_uint& rhs) const noexcept;
    bool operator<(const bf_uint& rhs) const noexcept;
    bool operator<=(const bf_uint& rhs) const noexcept;
    bool operator>(const bf_uint& rhs) const noexcept;
    bool operator>=(const bf_uint& rhs) const noexcept;

    // arithmetic
    bf_uint operator+(const bf_uint& rhs) const;
    bf_uint operator-(const bf_uint& rhs) const;
    bf_uint operator*(const bf_uint& rhs) const;
    bf_uint operator/(const bf_uint& rhs) const;
    bf_uint operator%(const bf_uint& rhs) const;

    // compound arithmetic
    bf_uint& operator+=(const bf_uint& rhs);
    bf_uint& operator-=(const bf_uint& rhs);
    bf_uint& operator*=(const bf_uint& rhs);
    bf_uint& operator/=(const bf_uint& rhs);
    bf_uint& operator%=(const bf_uint& rhs);

    // bitwise
    bf_uint operator&(const bf_uint& rhs) const;
    bf_uint operator|(const bf_uint& rhs) const;
    bf_uint operator^(const bf_uint& rhs) const;
    bf_uint operator~() const;

    // compound bitwise
    bf_uint& operator&=(const bf_uint& rhs);
    bf_uint& operator|=(const bf_uint& rhs);
    bf_uint& operator^=(const bf_uint& rhs);

    // shifts
    bf_uint Shl(uint32_t s) const;
    bf_uint Shr(uint32_t s) const;
    bf_uint operator<<(uint32_t s) const;
    bf_uint operator>>(uint32_t s) const;
    bf_uint operator<<(const bf_uint& rhs) const;
    bf_uint operator>>(const bf_uint& rhs) const;

    // compound shifts
    bf_uint& operator<<=(uint32_t s);
    bf_uint& operator>>=(uint32_t s);
    bf_uint& operator<<=(const bf_uint& rhs);
    bf_uint& operator>>=(const bf_uint& rhs);

    // rotates
    bf_uint RotL(uint32_t s) const;
    bf_uint RotR(uint32_t s) const;

    // unary
    bf_uint operator-() const;

  private:
    void Normalize();
};

} // namespace BitFlow::Core::BitVector