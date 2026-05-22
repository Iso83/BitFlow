#pragma once

#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/ExprRef.h>
#include <BitFlow/core/helper/Debug.h>

namespace BitFlow::Core::Rules {
class RuleEngine;
class RewriteContext;
} // namespace BitFlow::Core::Rules

namespace BitFlow::IO {
class PrattParser;
}

namespace BitFlow::Core::Expression {

class ExprStore {
    friend class Rules::RuleEngine;
    friend class Rules::RewriteContext;
    friend class BitFlow::IO::PrattParser;

  private:
#pragma region Storage
    using ValueType = Ids::ExprId::ValueType;

    ValueType m_nextId{1};
    std::vector<ValueType> m_freeIds{};

    Ids::ExprId m_zero;

#ifdef BF_EXPR_LIFETIME_CHECKS
    friend Expr;

    uint64_t m_generation{};

    uint32_t m_nextDebugSlot{};
    std::vector<Expr> m_debugExprs{};
    std::vector<ExprUnsafeStorage> m_nodes{};
#else
    std::vector<Expr> m_nodes{};
#endif
    std::vector<bool> m_alive{};
#pragma endregion

  public:
    ExprStore();
    ~ExprStore() = default;

#pragma region Constants
    [[nodiscard]] inline Ids::ExprId zeroId() const noexcept {
        return m_zero;
    }

    [[nodiscard]] inline ExprRef zero() const {
        return ExprRef(const_cast<ExprStore*>(this), m_zero);
    }

    [[nodiscard]] ExprRef createConstant(Types::ExprChunk value, Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] ExprRef makeFalse(Types::BitWidth bitWidth = Types::ExprChunkBits) {
        return createConstant(Types::ExprChunk{0}, bitWidth);
    }

    [[nodiscard]] bool isFalse(Ids::ExprId id) const {
        const Expr& e = get(id);

        BF_CORE_ASSERT(e.op == OpType::Const && e.inputs.empty());

        return e.knownValue == 0;
    }

    [[nodiscard]] ExprRef makeTrue(Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] bool isTrue(Ids::ExprId id) const {
        const Expr& e = get(id);

        BF_CORE_ASSERT(e.op == OpType::Const && e.inputs.empty());

        return e.knownValue == Expr::fullMask(e.bitWidth);
    }

    [[nodiscard]] ExprRef invertConst(Ids::ExprId id);
#pragma endregion

#pragma region Create
    [[nodiscard]] ExprRef create(OpType op, std::initializer_list<Ids::ExprId> in,
                                 Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] ExprRef create(OpType op, std::vector<Ids::ExprId>&& in,
                                 Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] ExprRef createVariable(Types::BitWidth bitWidth = Types::ExprChunkBits) {
        return create(OpType::Var, {}, bitWidth);
    }
#pragma endregion

#pragma region Query
    [[nodiscard]] bool remove(ExprRef ref);

    [[nodiscard]] bool contains(ExprRef ref) const;
#pragma endregion

#pragma region Operators
    [[nodiscard]] const Expr& operator[](Ids::ExprId id) const {
        return get(id);
    }

    [[nodiscard]] const Expr& operator[](ExprRef ref) const {
        return get(ref.id);
    }
#pragma endregion

#pragma endregion Equivalence
    [[nodiscard]] bool structuralEquivalent(Ids::ExprId a, Ids::ExprId b) const;
    [[nodiscard]] inline bool structuralEquivalent(ExprRef a, ExprRef b) const {
        return structuralEquivalent(a.id, b.id);
    }

    [[nodiscard]] bool equalConstValue(Ids::ExprId a, Ids::ExprId b) const;
    [[nodiscard]] inline bool equalConstValue(ExprRef a, ExprRef b) const {
        return equalConstValue(a.id, b.id);
    }
#pragma endregion

  private:
#pragma region Internal
    void ensureCapacity(size_t size) {
#ifdef BF_EXPR_LIFETIME_CHECKS
        m_generation++;
#endif
        m_nodes.resize(size);
        m_alive.resize(size, false);
    }

    [[nodiscard]] Ids::ExprId createId();

#ifdef BF_EXPR_LIFETIME_CHECKS
    [[nodiscard]] Expr& MakeDebugExpr(Ids::ExprId id);
#endif

    [[nodiscard]] Expr& get(Ids::ExprId id) {
#ifdef BF_EXPR_LIFETIME_CHECKS
        return MakeDebugExpr(id);
#else
        return m_nodes[toIndex(id)];
#endif
    }

    [[nodiscard]] static size_t toIndex(Ids::ExprId id) {
        return static_cast<size_t>(id.value());
    }

    [[nodiscard]] const Expr& get(Ids::ExprId id) const {
#ifdef BF_EXPR_LIFETIME_CHECKS
        return const_cast<ExprStore*>(this)->MakeDebugExpr(id);
#else
        return m_nodes[toIndex(id)];
#endif
    }

    void replace(Ids::ExprId oldId, Ids::ExprId newId) {}
    void release(Ids::ExprId oldId) {}
#pragma endregion
};

} // namespace BitFlow::Core::Expression