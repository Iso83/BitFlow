#include <BitFlow/core/bitvector/BitVector.h>

#include <algorithm>

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

} // namespace BitFlow::Core::BitVector
