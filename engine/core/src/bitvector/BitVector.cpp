#include <BitFlow/core/bitvector/BitVector.h>
#include <algorithm>
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

#pragma region string conversion
std::string bf_uint::ToString(StringBase base) const {
    switch (base) {
    case StringBase::Binary:
        return ToBinaryString();
    case StringBase::Decimal:
        return ToDecimalString();
    case StringBase::Hex:
        return ToHexString(false);
    default:
        return ToDecimalString();
    }
}

std::string bf_uint::ToBinaryString() const {
    if (m_bw == 0)
        return "0";

    std::string s;
    s.reserve(m_bw);

    bool started = false;

    for (uint32_t bit = m_bw; bit-- > 0;) {
        const size_t wi = bit / 64U;
        const uint32_t bi = bit % 64U;
        const bool set = ((m_words[wi] >> bi) & 1ULL) != 0ULL;

        if (set)
            started = true;

        if (started)
            s.push_back(set ? '1' : '0');
    }

    return s.empty() ? "0" : s;
}

std::string bf_uint::ToHexString(bool upperCase) const {
    static const char* lower = "0123456789abcdef";
    static const char* upper = "0123456789ABCDEF";
    const char* digits = upperCase ? upper : lower;

    if (m_bw == 0)
        return "0";

    const uint32_t hexDigits = (m_bw + 3) / 4;
    std::string s;
    s.reserve(hexDigits);

    bool started = false;

    for (uint32_t nib = hexDigits; nib-- > 0;) {
        uint32_t value = 0;

        for (uint32_t b = 0; b < 4; ++b) {
            const uint32_t bit = nib * 4 + b;
            if (bit >= m_bw)
                continue;

            const size_t wi = bit / 64U;
            const uint32_t bi = bit % 64U;

            value |= static_cast<uint32_t>(((m_words[wi] >> bi) & 1ULL) << b);
        }

        if (value != 0)
            started = true;

        if (started)
            s.push_back(digits[value]);
    }

    return s.empty() ? "0" : s;
}

std::string bf_uint::ToDecimalString() const {
    if (IsZero())
        return "0";

    bf_uint temp = *this;
    bf_uint ten(10, m_bw);

    std::string s;

    while (!temp.IsZero()) {
        bf_uint q = temp / ten;
        bf_uint r = temp % ten;

        s.push_back(static_cast<char>('0' + r.ToUint32()));
        temp = q;
    }

    std::reverse(s.begin(), s.end());
    return s;
}
#pragma endregion

#pragma region comparison
bool bf_uint::operator==(const bf_uint& rhs) const noexcept {
    return m_bw == rhs.m_bw && m_words == rhs.m_words;
}

bool bf_uint::operator!=(const bf_uint& rhs) const noexcept {
    return !(*this == rhs);
}

bool bf_uint::operator<(const bf_uint& rhs) const noexcept {
    if (m_bw != rhs.m_bw)
        return m_bw < rhs.m_bw;

    for (size_t i = m_words.size(); i-- > 0;) {
        if (m_words[i] < rhs.m_words[i])
            return true;
        if (m_words[i] > rhs.m_words[i])
            return false;
    }

    return false;
}

bool bf_uint::operator<=(const bf_uint& rhs) const noexcept {
    return !(*this > rhs);
}

bool bf_uint::operator>(const bf_uint& rhs) const noexcept {
    return rhs < *this;
}

bool bf_uint::operator>=(const bf_uint& rhs) const noexcept {
    return !(*this < rhs);
}
#pragma endregion

#pragma region arithmetic
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
        return bf_uint(0, 0);

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
        return bf_uint(0, 0);

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
        return bf_uint(0, 0);

    bf_uint q = (*this) / rhs;
    bf_uint prod = q * rhs;
    bf_uint rem = (*this) - prod;
    rem.Normalize();
    return rem;
}
#pragma endregion

#pragma region compound arithmetic
bf_uint& bf_uint::operator+=(const bf_uint& rhs) {
    *this = *this + rhs;
    return *this;
}

bf_uint& bf_uint::operator-=(const bf_uint& rhs) {
    *this = *this - rhs;
    return *this;
}

bf_uint& bf_uint::operator*=(const bf_uint& rhs) {
    *this = *this * rhs;
    return *this;
}

bf_uint& bf_uint::operator/=(const bf_uint& rhs) {
    *this = *this / rhs;
    return *this;
}

bf_uint& bf_uint::operator%=(const bf_uint& rhs) {
    *this = *this % rhs;
    return *this;
}
#pragma endregion

#pragma region bitwise
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
#pragma endregion

#pragma region compound bitwise
bf_uint& bf_uint::operator&=(const bf_uint& rhs) {
    *this = *this & rhs;
    return *this;
}

bf_uint& bf_uint::operator|=(const bf_uint& rhs) {
    *this = *this | rhs;
    return *this;
}

bf_uint& bf_uint::operator^=(const bf_uint& rhs) {
    *this = *this ^ rhs;
    return *this;
}
#pragma endregion

#pragma region shifts
bf_uint bf_uint::Shl(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;

    bf_uint r(m_bw);

    const uint32_t wordShift = s / 64;
    const uint32_t bitShift = s % 64;

    for (int i = static_cast<int>(m_words.size()) - 1; i >= 0; --i) {
        uint64_t val = 0;

        if (i - static_cast<int>(wordShift) >= 0) {
            val = m_words[static_cast<size_t>(i - static_cast<int>(wordShift))] << bitShift;

            if (bitShift && i - static_cast<int>(wordShift) - 1 >= 0)
                val |= m_words[static_cast<size_t>(i - static_cast<int>(wordShift) - 1)] >> (64 - bitShift);
        }

        r.m_words[static_cast<size_t>(i)] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::Shr(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;

    bf_uint r(m_bw);

    const uint32_t wordShift = s / 64;
    const uint32_t bitShift = s % 64;

    for (size_t i = 0; i < m_words.size(); ++i) {
        uint64_t val = 0;
        const size_t src = i + wordShift;

        if (src < m_words.size()) {
            val = m_words[src] >> bitShift;
            if (bitShift && (src + 1) < m_words.size())
                val |= m_words[src + 1] << (64 - bitShift);
        }

        r.m_words[i] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator<<(uint32_t s) const {
    return Shl(s);
}

bf_uint bf_uint::operator>>(uint32_t s) const {
    return Shr(s);
}

bf_uint bf_uint::operator<<(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);
    return Shl(rhs.ToUint32());
}

bf_uint bf_uint::operator>>(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);
    return Shr(rhs.ToUint32());
}
#pragma endregion

#pragma region compound shifts
bf_uint& bf_uint::operator<<=(uint32_t s) {
    *this = Shl(s);
    return *this;
}

bf_uint& bf_uint::operator>>=(uint32_t s) {
    *this = Shr(s);
    return *this;
}

bf_uint& bf_uint::operator<<=(const bf_uint& rhs) {
    *this = Shl(rhs.ToUint32());
    return *this;
}

bf_uint& bf_uint::operator>>=(const bf_uint& rhs) {
    *this = Shr(rhs.ToUint32());
    return *this;
}
#pragma endregion

#pragma region rotates
bf_uint bf_uint::RotL(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;
    return Shl(s) | Shr(m_bw - s);
}

bf_uint bf_uint::RotR(uint32_t s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;
    return Shr(s) | Shl(m_bw - s);
}
#pragma endregion

#pragma region unary
bf_uint bf_uint::operator-() const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    bf_uint zero(m_bw);
    return zero - *this;
}
#pragma endregion

void bf_uint::Normalize() {
    if (m_words.empty())
        return;

    const uint32_t rem = m_bw % 64;
    if (rem != 0)
        m_words.back() &= Mask64(rem);
}
} // namespace BitFlow::Core::BitVector