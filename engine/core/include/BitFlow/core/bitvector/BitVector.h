#pragma once

#include <BitFlow/core/types/Types.h>
#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::BitVector {

class bf_uint {
  private:
    Types::BitWidth m_bw = 0;
    std::vector<Types::ExprChunk> m_words;

  public:
    enum class StringBase : uint8_t { Binary = 2, Decimal = 10, Hex = 16 };

  public:
    explicit bf_uint(Types::BitWidth bitWidth);
    explicit bf_uint(Types::ExprChunk value, Types::BitWidth bitWidth);

    Types::BitWidth BitWidth() const noexcept {
        return m_bw;
    }

    bool IsZero() const noexcept {
        for (Types::ExprChunk w : m_words) {
            if (w != Types::ExprChunk{0})
                return false;
        }
        return true;
    }

    Types::ExprChunk ToChunk() const noexcept {
        return m_words.empty() ? Types::ExprChunk{0} : m_words[0];
    }
    uint64_t ToUint64() const noexcept {
        return static_cast<uint64_t>(ToChunk() & 0xffffffffffffffffULL);
    }
    uint32_t ToUint32() const noexcept {
        return static_cast<uint32_t>(ToChunk() & 0xffffffffULL);
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
    bf_uint Shl(Types::BitWidth s) const;
    bf_uint Shr(Types::BitWidth s) const;
    bf_uint operator<<(Types::BitWidth s) const;
    bf_uint operator>>(Types::BitWidth s) const;
    bf_uint operator<<(const bf_uint& rhs) const;
    bf_uint operator>>(const bf_uint& rhs) const;

    // compound shifts
    bf_uint& operator<<=(Types::BitWidth s);
    bf_uint& operator>>=(Types::BitWidth s);
    bf_uint& operator<<=(const bf_uint& rhs);
    bf_uint& operator>>=(const bf_uint& rhs);

    // rotates
    bf_uint RotL(Types::BitWidth s) const;
    bf_uint RotR(Types::BitWidth s) const;

    // unary
    bf_uint operator-() const;

  private:
    void Normalize();
};

} // namespace BitFlow::Core::BitVector