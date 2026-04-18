#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::BitVector {

class BitVector {
public:
    explicit BitVector(uint32_t bitWidth = 0);
    BitVector(uint32_t bitWidth, uint64_t value);

    [[nodiscard]] uint32_t BitWidth() const noexcept;
    [[nodiscard]] bool Empty() const noexcept;

    [[nodiscard]] uint64_t Low64() const noexcept;
    [[nodiscard]] std::string ToHexString() const;

    void Clear() noexcept;
    void SetFromU64(uint64_t value) noexcept;

    void NotInPlace() noexcept;
    void AndInPlace(const BitVector& rhs);
    void OrInPlace(const BitVector& rhs);
    void XorInPlace(const BitVector& rhs);

    void AddInPlace(const BitVector& rhs);
    void SubInPlace(const BitVector& rhs);

private:
    static constexpr uint32_t kWordBits = 64U;

    [[nodiscard]] size_t WordCount() const noexcept;
    void EnsureSameWidth(const BitVector& rhs) const;
    void TrimExcessBits() noexcept;

    uint32_t bitWidth_ = 0;
    std::vector<uint64_t> words_{};
};

} // namespace BitFlow::Core::BitVector
