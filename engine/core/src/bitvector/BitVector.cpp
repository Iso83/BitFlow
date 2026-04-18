#include <BitFlow/core/bitvector/BitVector.h>

#include <algorithm>
#include <cstddef>

namespace BitFlow::Core::BitVector {

static uint64_t Mask64(uint32_t bits) {
    if (bits == 64)
        return ~0ull;
    return (1ull << bits) - 1;
}

bf_uint::bf_uint(uint32_t bw)
    : m_bw(bw) {
    m_words.resize((bw + 63) / 64, 0);
}

bf_uint::bf_uint(uint64_t v, uint32_t bw)
    : bf_uint(bw) {
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
    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] & rhs.m_words[i];
    return r;
}

bf_uint bf_uint::operator|(const bf_uint& rhs) const {
    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] | rhs.m_words[i];
    return r;
}

bf_uint bf_uint::operator^(const bf_uint& rhs) const {
    bf_uint r(m_bw);
    for (size_t i = 0; i < m_words.size(); ++i)
        r.m_words[i] = m_words[i] ^ rhs.m_words[i];
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
                val |= m_words[static_cast<size_t>(i - static_cast<int>(wordShift) - 1)] >>
                       (64 - bitShift);
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

} // namespace BitFlow::Core::BitVector
