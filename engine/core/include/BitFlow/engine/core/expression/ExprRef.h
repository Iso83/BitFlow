#pragma once

#include <BitFlow/engine/core/ids/ExprId.h>
#include <BitFlow/engine/core/types/Types.h>

namespace BitFlow::Engine::Core::Expression {

class ExprStore;

struct ExprRef {
    ExprStore* store{};
    Ids::ExprId id{};

    ExprRef() = default;
    ExprRef(ExprStore* owner, Ids::ExprId exprId);

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] bool operator==(const ExprRef& other) const noexcept;
    [[nodiscard]] bool operator!=(const ExprRef& other) const noexcept;

    [[nodiscard]] Types::BitWidth BitWidth() const;

    [[nodiscard]] ExprRef Const(Types::ExprChunk value, Types::BitWidth bitWidth = 0) const;

    [[nodiscard]] ExprRef operator~() const;
    [[nodiscard]] ExprRef operator-() const;

    [[nodiscard]] ExprRef operator+(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator-(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator*(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator/(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator%(ExprRef rhs) const;

    [[nodiscard]] ExprRef Pow(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator<<(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator>>(ExprRef rhs) const;
    [[nodiscard]] ExprRef RotL(ExprRef rhs) const;
    [[nodiscard]] ExprRef RotR(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator&(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator^(ExprRef rhs) const;
    [[nodiscard]] ExprRef operator|(ExprRef rhs) const;

    [[nodiscard]] ExprRef operator+(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator-(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator*(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator/(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator%(Types::ExprChunk rhs) const;

    [[nodiscard]] ExprRef Pow(Types::ExprChunk exp) const;

    [[nodiscard]] ExprRef operator<<(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator>>(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef RotL(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef RotR(Types::ExprChunk rhs) const;

    [[nodiscard]] ExprRef operator&(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator^(Types::ExprChunk rhs) const;
    [[nodiscard]] ExprRef operator|(Types::ExprChunk rhs) const;
};

} // namespace BitFlow::Core::Expression
