#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <cstdint>

namespace BitFlow::Core::Expression {

class ExprStore;

struct ExprRef {
    ExprStore* store{};
    Ids::ExprId id{};

    inline static uint16_t defaultBitWidth{64};

    ExprRef() = default;
    ExprRef(ExprStore* owner, Ids::ExprId exprId);

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] bool operator==(const ExprRef& other) const noexcept;
    [[nodiscard]] bool operator!=(const ExprRef& other) const noexcept;

    [[nodiscard]] uint16_t BitWidth() const;

    [[nodiscard]] ExprRef Const(uint64_t value, uint16_t bitWidth = 0) const;

    [[nodiscard]] ExprRef operator~() const;
    [[nodiscard]] ExprRef operator-() const;

    [[nodiscard]] ExprRef operator+(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator-(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator*(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator/(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator%(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator<<(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator>>(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator&(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator^(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator|(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator+(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator-(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator*(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator/(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator%(uint64_t rhs) const;

    [[nodiscard]] ExprRef operator<<(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator>>(uint64_t rhs) const;

    [[nodiscard]] ExprRef operator&(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator^(uint64_t rhs) const;
    [[nodiscard]] ExprRef operator|(uint64_t rhs) const;
};

} // namespace BitFlow::Core::Expression