#pragma once

#include <cstdint>
#include <vector>

namespace BitFlow::Core::BitVector {

// Representatie:
// - little-endian words: m_words[0] bevat de LSB 64 bits
// - wordgrootte: 64-bit (uint64_t)
// - aantal words: ceil(bitWidth / 64)
class bf_uint {
public:
    bf_uint(uint32_t bitWidth);
    bf_uint(uint64_t value, uint32_t bitWidth);

    uint32_t BitWidth() const noexcept;

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

    // rotates
    bf_uint RotL(uint32_t s) const;
    bf_uint RotR(uint32_t s) const;

private:
    uint32_t m_bw;
    std::vector<uint64_t> m_words;

    void Normalize(); // mask toepassen
};

} // namespace BitFlow::Core::BitVector
