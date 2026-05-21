#include <BitFlow/core/expression/ExprRefUtils.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/helper/Exception.h>

namespace BitFlow::Core::Expression {

using namespace Ids;

ExprStore::ExprStore() {

#ifdef BF_EXPR_LIFETIME_CHECKS
    m_debugExprs.resize(200000);
#endif

    m_zero = createConstant(0, Types::BitWidth{1}).id;
}

#pragma region Constants
[[nodiscard]] ExprRef ExprStore::createConstant(Types::ExprChunk value, Types::BitWidth bitWidth) {
    auto ref = create(OpType::Const, {}, bitWidth);

    Expr& expr = get(ref.id);
    expr.knownMask = Expr::fullMask(bitWidth);
    expr.knownValue = value;

    return ref;
}

[[nodiscard]] ExprRef ExprStore::makeTrue(Types::BitWidth bitWidth) {
    auto ref = create(OpType::Const, {}, bitWidth);

    Expr& expr = get(ref.id);
    auto mask = Expr::fullMask(bitWidth);
    expr.knownMask = mask;
    expr.knownValue = mask;

    return ref;
}

[[nodiscard]] ExprRef ExprStore::invertConst(ExprId id) {
    const Expr& e = get(id);

    if (e.op != OpType::Const)
        BF_CORE_THROW("ExprStore::invertConst expects Const");

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);
    const Types::ExprChunk value = (~e.knownValue) & mask;

    return createConstant(value, e.bitWidth);
}
#pragma endregion

#pragma region Create
[[nodiscard]] ExprRef ExprStore::create(OpType op, std::initializer_list<ExprId> in, Types::BitWidth bitWidth) {
    BF_CORE_ASSERT(bitWidth > 0);

    const auto id = createId();
    const auto index = toIndex(id);

    ensureCapacity(index + 1);

    auto& expr = m_nodes[index];
#ifdef BF_EXPR_LIFETIME_CHECKS
    expr = ExprUnsafeStorage{};
#else
    expr = Expr{};
#endif
    expr.op = op;
    expr.bitWidth = bitWidth;
    expr.inputs = in;

    m_alive[index] = true;

#ifdef BF_EXPR_LIFETIME_CHECKS
    if (m_nextDebugSlot >= m_debugExprs.size())
        BF_CORE_THROW(
            "ExprStore debug wrapper overflow: m_debugExprs cannot grow because wrapper addresses must remain stable.");

    auto& debugExpr = m_debugExprs[m_nextDebugSlot++];
    debugExpr.m_store = this;
    debugExpr.m_id = id;
    debugExpr.m_expr = &expr;
    debugExpr.m_generation = m_generation;
#endif

    return ExprRef(this, id);
}

[[nodiscard]] ExprRef ExprStore::create(OpType op, std::vector<ExprId>&& in, Types::BitWidth bitWidth) {
    BF_CORE_ASSERT(bitWidth > 0);

    const auto id = createId();
    const auto index = toIndex(id);

    ensureCapacity(index + 1);

    auto& expr = m_nodes[index];
#ifdef BF_EXPR_LIFETIME_CHECKS
    expr = ExprUnsafeStorage{};
#else
    expr = Expr{};
#endif
    expr.op = op;
    expr.bitWidth = bitWidth;
    expr.inputs = std::move(in);

    m_alive[index] = true;

#ifdef BF_EXPR_LIFETIME_CHECKS
    if (m_nextDebugSlot >= m_debugExprs.size())
        BF_CORE_THROW(
            "ExprStore debug wrapper overflow: m_debugExprs cannot grow because wrapper addresses must remain stable.");

    auto& debugExpr = m_debugExprs[m_nextDebugSlot++];
    debugExpr.m_store = this;
    debugExpr.m_id = id;
    debugExpr.m_expr = &expr;
    debugExpr.m_generation = m_generation;
#endif

    return ExprRef(this, id);
}
#pragma endregion

#pragma region Query
[[nodiscard]] bool ExprStore::remove(ExprRef ref) {
    if (!contains(ref))
        return false;

    const auto index = toIndex(ref.id);

    m_alive[index] = false;
    m_freeIds.push_back(ref.id.value());

    return true;
}

[[nodiscard]] bool ExprStore::contains(ExprRef ref) const {
    if (ref.store != this || ref.id.value() == 0)
        return false;

    const auto index = toIndex(ref.id);

    if (index >= m_alive.size())
        return false;

    return m_alive[index];
}
#pragma endregion

void CollectEquivalentInputs(const ExprStore& store, ExprId id, OpType op, std::vector<ExprId>& out) {
    const Expr& e = store[id];

    if (e.op == op && IsAssociative(op)) {
        for (ExprId input : e.inputs)
            CollectEquivalentInputs(store, input, op, out);

        return;
    }

    out.push_back(id);
}

#pragma region Equivalence
[[nodiscard]] bool ExprStore::structuralEquivalent(ExprId a, ExprId b) const {
    // fast path
    if (a == b)
        return true;

    const Expr& ea = (*this)[a];
    const Expr& eb = (*this)[b];

    // basic structure
    if (ea.op != eb.op)
        return false;

    if (ea.bitWidth != eb.bitWidth)
        return false;

    if (ea.inputs.size() != eb.inputs.size())
        return false;

    // constants
    if (ea.op == OpType::Const)
        return equalConstValue(a, b);

    // variables
    if (ea.op == OpType::Var)
        return false;

    // no children
    if (ea.inputs.empty())
        return true;

    const bool commutative = IsCommutative(ea.op);
    const bool associative = IsAssociative(ea.op);

    // ordered compare
    if (!commutative) {
        for (size_t i = 0; i < ea.inputs.size(); ++i) {
            if (!structuralEquivalent(ea.inputs[i], eb.inputs[i]))
                return false;
        }

        return true;
    }

    std::vector<ExprId> lhsInputs;
    std::vector<ExprId> rhsInputs;

    if (associative) {
        CollectEquivalentInputs(*this, a, ea.op, lhsInputs);
        CollectEquivalentInputs(*this, b, eb.op, rhsInputs);
    } else {
        lhsInputs = ea.inputs;
        rhsInputs = eb.inputs;
    }

    if (lhsInputs.size() != rhsInputs.size())
        return false;

    // unordered compare for commutative ops
    std::vector<bool> matched(rhsInputs.size(), false);

    for (ExprId lhsInput : lhsInputs) {
        bool found = false;

        for (size_t j = 0; j < rhsInputs.size(); ++j) {
            if (matched[j])
                continue;

            if (!structuralEquivalent(lhsInput, rhsInputs[j]))
                continue;

            matched[j] = true;
            found = true;
            break;
        }

        if (!found)
            return false;
    }

    return true;
}

[[nodiscard]] bool ExprStore::equalConstValue(ExprId a, ExprId b) const {
    const Expr& ea = get(a);
    const Expr& eb = get(b);

    BF_CORE_ASSERT(ea.op == OpType::Const);
    BF_CORE_ASSERT(eb.op == OpType::Const);

    return EqualChunkValue(this, a, eb.knownValue);
}
#pragma endregion

#pragma region Internal
[[nodiscard]] ExprId ExprStore::createId() {
    ValueType value{};

    if (!m_freeIds.empty()) {
        value = m_freeIds.back();
        m_freeIds.pop_back();
    } else {
        value = m_nextId++;
    }

    return ExprId{value};
}

#ifdef BF_EXPR_LIFETIME_CHECKS
[[nodiscard]] Expr& ExprStore::MakeDebugExpr(ExprId id) {

    if (m_nextDebugSlot >= m_debugExprs.size())
        BF_CORE_THROW(
            "ExprStore debug wrapper overflow: m_debugExprs cannot grow because wrapper addresses must remain stable.");

    auto& debugExpr = m_debugExprs[m_nextDebugSlot++];
    debugExpr.m_store = this;
    debugExpr.m_id = id;
    debugExpr.m_expr = &m_nodes[toIndex(id)];
    debugExpr.m_generation = m_generation;

    return debugExpr;
}
#endif
#pragma endregion

} // namespace BitFlow::Core::Expression