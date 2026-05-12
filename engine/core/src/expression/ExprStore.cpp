#include <BitFlow/core/expression/ExprStore.h>
#include <stdexcept>

namespace BitFlow::Core::Expression {

using namespace Ids;

ExprStore::ExprStore() {
    m_zero = createConstant(0, Types::BitWidth{1}).id;

#ifdef BF_EXPR_LIFETIME_CHECKS
    m_debugExprs.resize(200000);
#endif
}

[[nodiscard]] ExprRef ExprStore::create(OpType op, std::initializer_list<ExprId> in, Types::BitWidth bitWidth) {
    _ASSERT(bitWidth > 0);

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
        throw std::runtime_error(
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
    _ASSERT(bitWidth > 0);

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
        throw std::runtime_error(
            "ExprStore debug wrapper overflow: m_debugExprs cannot grow because wrapper addresses must remain stable.");

    auto& debugExpr = m_debugExprs[m_nextDebugSlot++];
    debugExpr.m_store = this;
    debugExpr.m_id = id;
    debugExpr.m_expr = &expr;
    debugExpr.m_generation = m_generation;
#endif

    return ExprRef(this, id);
}

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
        throw std::runtime_error("ExprStore::invertConst expects Const");

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);
    const Types::ExprChunk value = (~e.knownValue) & mask;

    return createConstant(value, e.bitWidth);
}

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

#ifdef BF_EXPR_LIFETIME_CHECKS
[[nodiscard]] Expr& ExprStore::MakeDebugExpr(ExprId id) {

    if (m_nextDebugSlot >= m_debugExprs.size())
        throw std::runtime_error("ExprStore debug wrapper overflow: m_debugExprs cannot grow because wrapper "
                                 "addresses must remain stable.");

    auto& debugExpr = m_debugExprs[m_nextDebugSlot++];
    debugExpr.m_store = this;
    debugExpr.m_id = id;
    debugExpr.m_expr = &m_nodes[toIndex(id)];
    debugExpr.m_generation = m_generation;

    return debugExpr;
}
#endif

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

} // namespace BitFlow::Core::Expression