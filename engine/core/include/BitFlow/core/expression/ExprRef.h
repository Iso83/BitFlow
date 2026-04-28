#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <cstdint>

namespace BitFlow::Core::Expression {

class ExprStore;

struct ExprRef {
    ExprStore* store{};
    Ids::ExprId id{};
    uint32_t generation{0};

    ExprRef() = default;

    ExprRef(ExprStore* owner, Ids::ExprId exprId, uint32_t gen = 0) : store(owner), id(exprId), generation(gen) {}

    [[nodiscard]] bool IsValid() const noexcept {
        return store != nullptr && id.value() != 0;
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return IsValid();
    }

    [[nodiscard]] bool operator==(const ExprRef& other) const noexcept {
        return store == other.store && id == other.id && generation == other.generation;
    }

    [[nodiscard]] bool operator!=(const ExprRef& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] bool SameStore(const ExprRef& other) const noexcept {
        return store == other.store;
    }
};

} // namespace BitFlow::Core::Expression