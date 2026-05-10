#pragma once

#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/ExprRef.h>

namespace BitFlow::Core::Expression {

class ExprStore {
  private:
    using ValueType = Ids::ExprId::ValueType;

    ValueType m_nextId{1};
    std::vector<ValueType> m_freeIds{};

#ifdef BF_EXPR_LIFETIME_CHECKS
    friend Expr;

    uint64_t m_generation{};

    uint32_t m_nextDebugSlot{};
    std::vector<Expr> m_debugExprs{};
    std::vector<_Expr_INTERNALONLY> m_nodes{};
#else
    std::vector<Expr> m_nodes{};
#endif
    std::vector<bool> m_alive{};

  public:
#ifdef BF_EXPR_LIFETIME_CHECKS
    ExprStore();
#else
    ExprStore() = default;
#endif
    ~ExprStore() = default;

    [[nodiscard]] ExprRef create(OpType op, std::initializer_list<Ids::ExprId> in,
                                 Types::BitWidth bitWidth = Types::ExprChunkBits);
    [[nodiscard]] ExprRef create(OpType op, std::vector<Ids::ExprId>&& in,
                                 Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] ExprRef createConstant(Types::ExprChunk value, Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] ExprRef createVariable(Types::BitWidth bitWidth = Types::ExprChunkBits) {
        return create(OpType::Var, {}, bitWidth);
    }

    [[nodiscard]] ExprRef makeFalse(Types::BitWidth bitWidth = Types::ExprChunkBits) {
        return createConstant(Types::ExprChunk{0}, bitWidth);
    }

    [[nodiscard]] bool isFalse(Ids::ExprId id) const {
        const Expr& e = get(id);

        _ASSERT(e.op == OpType::Const && e.inputs.empty());

        return e.knownValue == 0;
    }

    [[nodiscard]] ExprRef makeTrue(Types::BitWidth bitWidth = Types::ExprChunkBits);

    [[nodiscard]] bool isTrue(Ids::ExprId id) const {
        const Expr& e = get(id);

        _ASSERT(e.op == OpType::Const && e.inputs.empty());

        return e.knownValue == Expr::fullMask(e.bitWidth);
    }

    [[nodiscard]] ExprRef invertConst(Ids::ExprId id);

    [[nodiscard]] bool remove(ExprRef ref);

    [[nodiscard]] bool contains(ExprRef ref) const;

    [[nodiscard]] Expr& get(ExprRef ref) {
#ifdef BF_EXPR_LIFETIME_CHECKS
        return get(ref.id);
#else
        return m_nodes[toIndex(ref.id)];
#endif
    }

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

    [[nodiscard]] const Expr& get(ExprRef ref) const {
#ifdef BF_EXPR_LIFETIME_CHECKS
        return get(ref.id);
#else
        return m_nodes[toIndex(ref.id)];
#endif
    }

    [[nodiscard]] const Expr& get(Ids::ExprId id) const {
#ifdef BF_EXPR_LIFETIME_CHECKS
        return const_cast<ExprStore*>(this)->MakeDebugExpr(id);
#else
        return m_nodes[toIndex(id)];
#endif
    }

    [[nodiscard]] const Expr& operator[](ExprRef ref) const {
        return get(ref);
    }

  private:
    [[nodiscard]] Ids::ExprId createId();

    [[nodiscard]] static size_t toIndex(Ids::ExprId id) {
        return static_cast<size_t>(id.value());
    }

    void ensureCapacity(size_t size) {
#ifdef BF_EXPR_LIFETIME_CHECKS
        m_generation++;
#endif
        m_nodes.resize(size);
        m_alive.resize(size, false);
    }
};

} // namespace BitFlow::Core::Expression