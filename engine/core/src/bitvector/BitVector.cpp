#include <BitFlow/core/bitvector/BitVector.h>
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace BitFlow::Core::BitVector {

static uint64_t Mask64(uint32_t bits) {
    if (bits == 64)
        return ~0ull;
    return (1ull << bits) - 1;
}

static void EnsureSameBitWidth(const bf_uint& lhs, const bf_uint& rhs) {
    if (lhs.BitWidth() != rhs.BitWidth())
        throw std::invalid_argument("bf_uint width mismatch");
}

bf_uint::bf_uint(uint32_t bw) : m_bw(bw) {
    m_words.resize((bw + 63) / 64, 0);
}

bf_uint::bf_uint(uint64_t v, uint32_t bw) : bf_uint(bw) {
    if (!m_words.empty())
        m_words[0] = v;
    Normalize();
}

uint32_t bf_uint::BitWidth() const noexcept {
    return m_bw;
}

void bf_uint::Normalize() {
    if (m_words.empty())
        return;

    const uint32_t rem = m_bw % 64;
    if (rem != 0) {
        m_words.back() &= Mask64(rem);
    }
}

bf_uint bf_uint::operator&(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] & rhs.m_words[i];
    r.Normalize();
    return r;
}

bf_uint bf_uint::operator|(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] | rhs.m_words[i];
    r.Normalize();
    return r;
}

bf_uint bf_uint::operator^(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] ^ rhs.m_words[i];
    r.Normalize();
    return r;
}

bf_uint bf_uint::operator~() const {
    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = ~m_words[i];
    r.Normalize();
    return r;
}

bf_uint bf_uint::Shl(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0);

    s %= m_bw;

    bf_uint r(m_bw);

    const uint32_t wordShift = s / 64;
    const uint32_t bitShift = s % 64;

    for (int i = static_cast<int>(m_words.size()) - 1; i >= 0; --i) {
        uint64_t val = 0;

        if (i - static_cast<int>(wordShift) >= 0) {
            val = m_words[static_cast<size_t>(i - static_cast<int>(wordShift))] << bitShift;

            if (bitShift && i - static_cast<int>(wordShift) - 1 >= 0) {
                val |= m_words[static_cast<size_t>(i - static_cast<int>(wordShift) - 1)] >> (64 - bitShift);
            }
        }

        r.m_words[static_cast<size_t>(i)] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::Shr(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0);

    s %= m_bw;

    bf_uint r(m_bw);

    const uint32_t wordShift = s / 64;
    const uint32_t bitShift = s % 64;

    for (size_t i = 0; i < m_words.size(); ++i) {
        uint64_t val = 0;
        const size_t src = i + wordShift;

        if (src < m_words.size()) {
            val = m_words[src] >> bitShift;
            if (bitShift && (src + 1) < m_words.size()) {
                val |= m_words[src + 1] << (64 - bitShift);
            }
        }

        r.m_words[i] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::RotL(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0);

    s %= m_bw;
    return Shl(s) | Shr(m_bw - s);
}

bf_uint bf_uint::RotR(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0);

    s %= m_bw;
    return Shr(s) | Shl(m_bw - s);
}

bf_uint bf_uint::operator+(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);

    uint64_t carry = 0;

    for (size_t i = 0; i < m_words.size(); ++i) {
        uint64_t a = m_words[i];
        uint64_t b = rhs.m_words[i];

        uint64_t sum = a + b + carry;

        carry = (sum < a) || (carry && sum == a);
        r.m_words[i] = sum;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator-(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);
    uint64_t borrow = 0;

    for (size_t i = 0; i < m_words.size(); ++i) {
        uint64_t a = m_words[i];
        uint64_t b = rhs.m_words[i];
        uint64_t sub = b + borrow;

        r.m_words[i] = a - sub;
        borrow = (a < sub) || (borrow && sub == 0);
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator*(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    if (m_bw == 0)
        return bf_uint(0);

    bf_uint r(m_bw);
    for (uint32_t bit = 0; bit < m_bw; ++bit) {
        const size_t wi = bit / 64U;
        const uint32_t bi = bit % 64U;
        const uint64_t set = (rhs.m_words[wi] >> bi) & 1ULL;
        if (set != 0ULL)
            r = r + Shl(bit);
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator/(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bool rhsIsZero = true;
    for (uint64_t w : rhs.m_words) {
        if (w != 0) {
            rhsIsZero = false;
            break;
        }
    }
    if (rhsIsZero)
        throw std::domain_error("bf_uint division by zero");

    if (m_bw == 0)
        return bf_uint(0);

    bf_uint q(m_bw);
    bf_uint rem(m_bw);

    for (uint32_t bit = m_bw; bit-- > 0;) {
        rem = rem.Shl(1);

        const size_t wi = bit / 64U;
        const uint32_t bi = bit % 64U;
        rem.m_words[0] |= (m_words[wi] >> bi) & 1ULL;

        bool ge = true;
        for (size_t i = rem.m_words.size(); i-- > 0;) {
            if (rem.m_words[i] < rhs.m_words[i]) {
                ge = false;
                break;
            }
            if (rem.m_words[i] > rhs.m_words[i]) {
                ge = true;
                break;
            }
        }

        if (ge) {
            rem = rem - rhs;
            q.m_words[wi] |= (1ULL << bi);
        }
    }

    q.Normalize();
    return q;
}

bf_uint bf_uint::operator%(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bool rhsIsZero = true;
    for (uint64_t w : rhs.m_words) {
        if (w != 0) {
            rhsIsZero = false;
            break;
        }
    }
    if (rhsIsZero)
        throw std::domain_error("bf_uint modulo by zero");

    if (m_bw == 0)
        return bf_uint(0);

    bf_uint q = (*this) / rhs;
    bf_uint prod = q * rhs;
    bf_uint rem = (*this) - prod;
    rem.Normalize();
    return rem;
}

} // namespace BitFlow::Core::BitVector
