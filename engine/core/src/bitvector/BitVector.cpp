#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/helper/Debug.h>
#include <BitFlow/core/helper/Exception.h>
#include <algorithm>

namespace BitFlow::Core::BitVector {

static Types::ExprChunk MaskChunk(Types::BitWidth bits) {
    if (bits == 0)
        return Types::ExprChunk{0};

    if (bits >= Types::ExprChunkBits)
        return ~Types::ExprChunk{0};

    return (Types::ExprChunk{1} << bits) - Types::ExprChunk{1};
}

static void EnsureSameBitWidth(const bf_uint& lhs, const bf_uint& rhs) {
    if (lhs.BitWidth() != rhs.BitWidth())
        BF_CORE_THROW_INVALID_ARGS("bf_uint width mismatch");
}

bf_uint::bf_uint(Types::BitWidth bw) : m_bw(bw) {
    m_words.resize((bw + Types::ExprChunkBitsMinusOne) / Types::ExprChunkBits, 0);
}

bf_uint::bf_uint(Types::ExprChunk v, Types::BitWidth bw) : bf_uint(bw) {
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

    for (Types::BitWidth bit = m_bw; bit-- > 0;) {
        const size_t wi = bit / Types::ExprChunkBits;
        const Types::BitWidth bi = bit % Types::ExprChunkBits;
        const bool set = ((m_words[wi] >> bi) & Types::ExprChunk{1}) != Types::ExprChunk{0};

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

    const Types::BitWidth hexDigits = (m_bw + 3) / 4;
    std::string s;
    s.reserve(hexDigits);

    bool started = false;

    for (Types::BitWidth nib = hexDigits; nib-- > 0;) {
        Types::BitWidth value = 0;

        for (Types::BitWidth b = 0; b < 4; ++b) {
            const Types::BitWidth bit = nib * 4 + b;
            if (bit >= m_bw)
                continue;

            const size_t wi = bit / Types::ExprChunkBits;
            const Types::BitWidth bi = bit % Types::ExprChunkBits;

            value |= static_cast<Types::BitWidth>(((m_words[wi] >> bi) & Types::ExprChunk{1}) << b);
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

    Types::ExprChunk carry = 0;

    for (size_t i = 0; i < m_words.size(); ++i) {
        Types::ExprChunk a = m_words[i];
        Types::ExprChunk b = rhs.m_words[i];

        Types::ExprChunk sum = a + b + carry;

        carry = (sum < a) || (carry && sum == a);
        r.m_words[i] = sum;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator-(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bf_uint r(m_bw);
    Types::ExprChunk borrow = 0;

    for (size_t i = 0; i < m_words.size(); ++i) {
        Types::ExprChunk a = m_words[i];
        Types::ExprChunk b = rhs.m_words[i];
        Types::ExprChunk sub = b + borrow;

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
    for (Types::BitWidth bit = 0; bit < m_bw; ++bit) {
        const size_t wi = bit / Types::ExprChunkBits;
        const Types::BitWidth bi = bit % Types::ExprChunkBits;
        const Types::ExprChunk set = (rhs.m_words[wi] >> bi) & Types::ExprChunk{1};
        if (set != Types::ExprChunk{0})
            r = r + Shl(bit);
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator/(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bool rhsIsZero = true;
    for (Types::ExprChunk w : rhs.m_words) {
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

    for (Types::BitWidth bit = m_bw; bit-- > 0;) {
        rem = rem.Shl(1);

        const size_t wi = bit / Types::ExprChunkBits;
        const Types::BitWidth bi = bit % Types::ExprChunkBits;
        rem.m_words[0] |= (m_words[wi] >> bi) & Types::ExprChunk{1};

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
            q.m_words[wi] |= (Types::ExprChunk{1} << bi);
        }
    }

    q.Normalize();
    return q;
}

bf_uint bf_uint::operator%(const bf_uint& rhs) const {
    EnsureSameBitWidth(*this, rhs);

    bool rhsIsZero = true;
    for (Types::ExprChunk w : rhs.m_words) {
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

bf_uint bf_uint::Pow(const Types::ExprChunk& rhs) const {
    bf_uint result(1, m_bw);

    if (rhs == 0)
        return result;

    bf_uint base = *this;
    Types::ExprChunk exp = rhs;

    while (exp > 0) {
        if (exp & 1)
            result *= base;

        exp >>= 1;

        if (exp != 0)
            base *= base;
    }

    result.Normalize();
    return result;
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
bf_uint bf_uint::Shl(Types::BitWidth s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    if (s >= m_bw)
        return bf_uint(0, m_bw);

    bf_uint r(m_bw);

    const Types::BitWidth wordShift = s / Types::ExprChunkBits;
    const Types::BitWidth bitShift = s % Types::ExprChunkBits;

    for (int i = static_cast<int>(m_words.size()) - 1; i >= 0; --i) {
        Types::ExprChunk val = 0;

        if (i - static_cast<int>(wordShift) >= 0) {
            val = m_words[static_cast<size_t>(i - static_cast<int>(wordShift))] << bitShift;

            if (bitShift && i - static_cast<int>(wordShift) - 1 >= 0)
                val |= m_words[static_cast<size_t>(i - static_cast<int>(wordShift) - 1)] >>
                       (Types::ExprChunkBits - bitShift);
        }

        r.m_words[static_cast<size_t>(i)] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::Shr(Types::BitWidth s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    if (s >= m_bw)
        return bf_uint(0, m_bw);

    bf_uint r(m_bw);

    const Types::BitWidth wordShift = s / Types::ExprChunkBits;
    const Types::BitWidth bitShift = s % Types::ExprChunkBits;

    for (size_t i = 0; i < m_words.size(); ++i) {
        Types::ExprChunk val = 0;
        const size_t src = i + wordShift;

        if (src < m_words.size()) {
            val = m_words[src] >> bitShift;
            if (bitShift && (src + 1) < m_words.size())
                val |= m_words[src + 1] << (Types::ExprChunkBits - bitShift);
        }

        r.m_words[i] = val;
    }

    r.Normalize();
    return r;
}

bf_uint bf_uint::operator<<(Types::BitWidth s) const {
    return Shl(s);
}

bf_uint bf_uint::operator>>(Types::BitWidth s) const {
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
bf_uint& bf_uint::operator<<=(Types::BitWidth s) {
    *this = Shl(s);
    return *this;
}

bf_uint& bf_uint::operator>>=(Types::BitWidth s) {
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
bf_uint bf_uint::RotL(Types::BitWidth s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;
    if (s == 0)
        return *this;

    return Shl(s) | Shr(m_bw - s);
}

bf_uint bf_uint::RotR(Types::BitWidth s) const {
    if (m_bw == 0)
        return bf_uint(0, 0);

    s %= m_bw;
    if (s == 0)
        return *this;

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

    const Types::BitWidth rem = m_bw % Types::ExprChunkBits;
    if (rem != 0)
        m_words.back() &= MaskChunk(rem);
}
} // namespace BitFlow::Core::BitVector