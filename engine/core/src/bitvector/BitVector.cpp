#include "bitvector/BitVector.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace BitFlow::Core::BitVector {

namespace {

size_t ComputeWordCount(uint32_t bitWidth) {
    return static_cast<size_t>((bitWidth + 63U) / 64U);
}

} // namespace

BitVector::BitVector(uint32_t bitWidth)
    : bitWidth_(bitWidth), words_(ComputeWordCount(bitWidth), 0ULL) {}

BitVector::BitVector(uint32_t bitWidth, uint64_t value)
    : BitVector(bitWidth) {
    SetFromU64(value);
}

uint32_t BitVector::BitWidth() const noexcept {
    return bitWidth_;
}

bool BitVector::Empty() const noexcept {
    return bitWidth_ == 0;
}

uint64_t BitVector::Low64() const noexcept {
    if (words_.empty())
        return 0ULL;

    return words_[0];
}

std::string BitVector::ToHexString() const {
    if (words_.empty())
        return "0x0";

    std::ostringstream oss;
    oss << "0x";

    const size_t last = words_.size() - 1;
    oss << std::hex << std::uppercase << words_[last];

    for (size_t i = last; i-- > 0;) {
        oss << std::setfill('0') << std::setw(16) << std::hex << std::uppercase << words_[i];
    }

    return oss.str();
}

void BitVector::Clear() noexcept {
    for (uint64_t& word : words_)
        word = 0ULL;
}

void BitVector::SetFromU64(uint64_t value) noexcept {
    if (words_.empty())
        return;

    words_[0] = value;
    for (size_t i = 1; i < words_.size(); ++i)
        words_[i] = 0ULL;

    TrimExcessBits();
}

void BitVector::NotInPlace() noexcept {
    for (uint64_t& word : words_)
        word = ~word;

    TrimExcessBits();
}

void BitVector::AndInPlace(const BitVector& rhs) {
    EnsureSameWidth(rhs);

    for (size_t i = 0; i < words_.size(); ++i)
        words_[i] &= rhs.words_[i];
}

void BitVector::OrInPlace(const BitVector& rhs) {
    EnsureSameWidth(rhs);

    for (size_t i = 0; i < words_.size(); ++i)
        words_[i] |= rhs.words_[i];
}

void BitVector::XorInPlace(const BitVector& rhs) {
    EnsureSameWidth(rhs);

    for (size_t i = 0; i < words_.size(); ++i)
        words_[i] ^= rhs.words_[i];

    TrimExcessBits();
}

void BitVector::AddInPlace(const BitVector& rhs) {
    EnsureSameWidth(rhs);

    uint64_t carry = 0ULL;
    for (size_t i = 0; i < words_.size(); ++i) {
        const uint64_t a = words_[i];
        const uint64_t b = rhs.words_[i];

        const uint64_t sum1 = a + b;
        const uint64_t carry1 = (sum1 < a) ? 1ULL : 0ULL;

        const uint64_t sum2 = sum1 + carry;
        const uint64_t carry2 = (sum2 < sum1) ? 1ULL : 0ULL;

        words_[i] = sum2;
        carry = (carry1 | carry2);
    }

    TrimExcessBits();
}

void BitVector::SubInPlace(const BitVector& rhs) {
    EnsureSameWidth(rhs);

    uint64_t borrow = 0ULL;
    for (size_t i = 0; i < words_.size(); ++i) {
        const uint64_t a = words_[i];
        const uint64_t b = rhs.words_[i] + borrow;

        const uint64_t bCarry = (b < rhs.words_[i]) ? 1ULL : 0ULL;
        words_[i] = a - b;

        const uint64_t underflow = (a < b) ? 1ULL : 0ULL;
        borrow = (underflow | bCarry);
    }

    TrimExcessBits();
}

size_t BitVector::WordCount() const noexcept {
    return words_.size();
}

void BitVector::EnsureSameWidth(const BitVector& rhs) const {
    if (bitWidth_ != rhs.bitWidth_) {
        throw std::invalid_argument("BitVector width mismatch");
    }
}

void BitVector::TrimExcessBits() noexcept {
    if (words_.empty())
        return;

    const uint32_t remainder = bitWidth_ % kWordBits;
    if (remainder == 0U)
        return;

    const uint64_t mask = (uint64_t{1} << remainder) - 1ULL;
    words_.back() &= mask;
}

} // namespace BitFlow::Core::BitVector
