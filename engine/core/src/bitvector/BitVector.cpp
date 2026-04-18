#include "bitvector/BitVector.h"

#include <stdexcept>

namespace BitFlow::Core::BitVector {
namespace {

size_t WordCountFor(uint32_t bw) {
    return static_cast<size_t>((bw + 63U) / 64U);
}

void EnsureCompatible(const bf_uint& lhs, const bf_uint& rhs) {
    if (lhs.BitWidth() != rhs.BitWidth())
        throw std::invalid_argument("bf_uint width mismatch");
}

uint32_t NormalizeShift(uint32_t shift, uint32_t bw) {
    if (bw == 0U)
        return 0U;

    return shift % bw;
}

} // namespace

bf_uint::bf_uint(uint32_t bitWidth)
    : m_bw(bitWidth), m_words(WordCountFor(bitWidth), 0ULL) {}

bf_uint::bf_uint(uint64_t value, uint32_t bitWidth)
    : bf_uint(bitWidth) {
    if (!m_words.empty())
        m_words[0] = value;

    Normalize();
}

uint32_t bf_uint::BitWidth() const noexcept {
    return m_bw;
}

bf_uint bf_uint::operator+(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    uint64_t carry = 0ULL;
    for (size_t i = 0; i < m_words.size(); ++i) {
        const uint64_t sum1 = m_words[i] + rhs.m_words[i];
        const uint64_t carry1 = (sum1 < m_words[i]) ? 1ULL : 0ULL;
        const uint64_t sum2 = sum1 + carry;
        const uint64_t carry2 = (sum2 < sum1) ? 1ULL : 0ULL;

        out.m_words[i] = sum2;
        carry = (carry1 | carry2);
    }

    out.Normalize();
    return out;
}

bf_uint bf_uint::operator-(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    uint64_t borrow = 0ULL;
    for (size_t i = 0; i < m_words.size(); ++i) {
        const uint64_t sub = rhs.m_words[i] + borrow;
        const uint64_t subCarry = (sub < rhs.m_words[i]) ? 1ULL : 0ULL;

        out.m_words[i] = m_words[i] - sub;
        const uint64_t underflow = (m_words[i] < sub) ? 1ULL : 0ULL;
        borrow = (underflow | subCarry);
    }

    out.Normalize();
    return out;
}

bf_uint bf_uint::operator*(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i) {
        uint64_t carry = 0ULL;
        for (size_t j = 0; j < rhs.m_words.size(); ++j) {
            const size_t k = i + j;
            if (k >= out.m_words.size())
                break;

            const unsigned __int128 prod = static_cast<unsigned __int128>(m_words[i]) *
                                           static_cast<unsigned __int128>(rhs.m_words[j]);
            const unsigned __int128 acc = static_cast<unsigned __int128>(out.m_words[k]) +
                                          static_cast<uint64_t>(prod) + carry;

            out.m_words[k] = static_cast<uint64_t>(acc);
            carry = static_cast<uint64_t>((prod >> 64U) + (acc >> 64U));
        }
    }

    out.Normalize();
    return out;
}

bf_uint bf_uint::operator/(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    const bf_uint zero(m_bw);
    if (rhs.m_words == zero.m_words)
        throw std::domain_error("bf_uint division by zero");

    bf_uint remainder(m_bw);
    bf_uint quotient(m_bw);

    for (uint32_t bit = m_bw; bit-- > 0;) {
        remainder = remainder.Shl(1);

        const uint32_t word = bit / 64U;
        const uint32_t offset = bit % 64U;
        const uint64_t bitValue = (m_words[word] >> offset) & 1ULL;
        if (!remainder.m_words.empty())
            remainder.m_words[0] |= bitValue;

        // if remainder >= rhs then subtract and set quotient bit
        bool ge = false;
        for (size_t i = remainder.m_words.size(); i-- > 0;) {
            if (remainder.m_words[i] > rhs.m_words[i]) {
                ge = true;
                break;
            }

            if (remainder.m_words[i] < rhs.m_words[i]) {
                ge = false;
                goto done_compare;
            }
        }
        ge = true;

    done_compare:
        if (ge) {
            remainder = remainder - rhs;
            quotient.m_words[word] |= (1ULL << offset);
        }
    }

    quotient.Normalize();
    return quotient;
}

bf_uint bf_uint::operator%(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    const bf_uint zero(m_bw);
    if (rhs.m_words == zero.m_words)
        throw std::domain_error("bf_uint modulo by zero");

    bf_uint quotient = (*this) / rhs;
    bf_uint product = quotient * rhs;
    bf_uint remainder = (*this) - product;
    remainder.Normalize();
    return remainder;
}

bf_uint bf_uint::operator&(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        out.m_words[i] = m_words[i] & rhs.m_words[i];

    return out;
}

bf_uint bf_uint::operator|(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        out.m_words[i] = m_words[i] | rhs.m_words[i];

    return out;
}

bf_uint bf_uint::operator^(const bf_uint& rhs) const {
    EnsureCompatible(*this, rhs);

    bf_uint out(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        out.m_words[i] = m_words[i] ^ rhs.m_words[i];

    out.Normalize();
    return out;
}

bf_uint bf_uint::operator~() const {
    bf_uint out(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        out.m_words[i] = ~m_words[i];

    out.Normalize();
    return out;
}

bf_uint bf_uint::Shl(uint32_t s) const {
    bf_uint out(m_bw);
    if (m_bw == 0U)
        return out;

    const uint32_t shift = NormalizeShift(s, m_bw);
    const uint32_t wordShift = shift / 64U;
    const uint32_t bitShift = shift % 64U;

    for (size_t i = m_words.size(); i-- > 0;) {
        if (i < wordShift)
            continue;

        uint64_t value = m_words[i - wordShift] << bitShift;
        if (bitShift != 0U && i > wordShift)
            value |= (m_words[i - wordShift - 1] >> (64U - bitShift));

        out.m_words[i] = value;
    }

    out.Normalize();
    return out;
}

bf_uint bf_uint::Shr(uint32_t s) const {
    bf_uint out(m_bw);
    if (m_bw == 0U)
        return out;

    const uint32_t shift = NormalizeShift(s, m_bw);
    const uint32_t wordShift = shift / 64U;
    const uint32_t bitShift = shift % 64U;

    for (size_t i = 0; i < m_words.size(); ++i) {
        const size_t src = i + wordShift;
        if (src >= m_words.size())
            break;

        uint64_t value = m_words[src] >> bitShift;
        if (bitShift != 0U && src + 1 < m_words.size())
            value |= (m_words[src + 1] << (64U - bitShift));

        out.m_words[i] = value;
    }

    out.Normalize();
    return out;
}

bf_uint bf_uint::RotL(uint32_t s) const {
    if (m_bw == 0U)
        return bf_uint(0U);

    const uint32_t shift = NormalizeShift(s, m_bw);
    if (shift == 0U)
        return *this;

    return Shl(shift) | Shr(m_bw - shift);
}

bf_uint bf_uint::RotR(uint32_t s) const {
    if (m_bw == 0U)
        return bf_uint(0U);

    const uint32_t shift = NormalizeShift(s, m_bw);
    if (shift == 0U)
        return *this;

    return Shr(shift) | Shl(m_bw - shift);
}

void bf_uint::Normalize() {
    if (m_words.empty())
        return;

    const uint32_t remainder = m_bw % 64U;
    if (remainder == 0U)
        return;

    const uint64_t mask = (uint64_t{1} << remainder) - 1ULL;
    m_words.back() &= mask;
}

} // namespace BitFlow::Core::BitVector
