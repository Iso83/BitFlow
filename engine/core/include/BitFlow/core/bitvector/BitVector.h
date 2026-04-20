#pragma once

#include <cstdint>
#include <vector>

namespace BitFlow::Core::BitVector {

class bf_uint {
  public:
    bf_uint(uint32_t bitWidth);
    bf_uint(uint64_t value, uint32_t bitWidth);

    uint32_t BitWidth() const noexcept;
    bool IsZero() const noexcept;
    uint64_t ToUint64() const noexcept;
    uint32_t ToUint32() const noexcept;

    // arithmetic
    bf_uint operator+(const bf_uint& rhs) const;
    bf_uint operator-(const bf_uint& rhs) const;
    bf_uint operator*(const bf_uint& rhs) const;
    bf_uint operator/(const bf_uint& rhs) const;
    bf_uint operator%(const bf_uint& rhs) const;

    // bitwise
    bf_uint operator&(const bf_uint& rhs) const;
    bf_uint operator|(const bf_uint& rhs) const;
    bf_uint operator^(const bf_uint& rhs) const;
    bf_uint operator~() const;

    // shifts
    bf_uint Shl(uint32_t s) const;
    bf_uint Shr(uint32_t s) const;
    bf_uint operator<<(uint32_t s) const;
    bf_uint operator>>(uint32_t s) const;
    bf_uint operator<<(const bf_uint& rhs) const;
    bf_uint operator>>(const bf_uint& rhs) const;

    // rotates
    bf_uint RotL(uint32_t s) const;
    bf_uint RotR(uint32_t s) const;
    bf_uint operator-() const;

  private:
    uint32_t m_bw = 0;
    std::vector<uint64_t> m_words;

    void Normalize(); // mask toepassen
};

} // namespace BitFlow::Core::BitVector
