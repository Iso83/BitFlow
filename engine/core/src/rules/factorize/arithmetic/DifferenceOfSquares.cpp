#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

namespace {

bool IsSquarePow(const ExprStore* store, ExprId id, ExprId& baseOut) {
    const Expr& expr = (*store)[id];
    if (expr.op != OpType::Pow || expr.inputs.size() != 2)
        return false;

    const Expr& exponent = (*store)[expr.inputs[1]];
    if (exponent.op != OpType::Const || exponent.knownValue != 2)
        return false;

    baseOut = expr.inputs[0];
    return true;
}

bool TryMatch(const ExprStore* store, ExprId id, ExprId& lhsBaseOut, ExprId& rhsBaseOut) {
    const Expr& expr = (*store)[id];
    if (expr.op != OpType::Sub || expr.inputs.size() != 2)
        return false;

    return IsSquarePow(store, expr.inputs[0], lhsBaseOut) && IsSquarePow(store, expr.inputs[1], rhsBaseOut);
}

} // namespace

static bool Match_DifferenceOfSquares(const ExprStore* store, ExprId id) {
    ExprId lhsBase{};
    ExprId rhsBase{};
    return TryMatch(store, id, lhsBase, rhsBase);
}

static ExprId Rewrite_DifferenceOfSquares(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    ExprId lhsBase{};
    ExprId rhsBase{};

    BF_CORE_ASSERT(TryMatch(store, id, lhsBase, rhsBase));

    const Types::BitWidth bitWidth = (*store)[id].bitWidth;

    ExprId diff = store->create(OpType::Sub, {lhsBase, rhsBase}, bitWidth).id;
    ExprId sum = store->create(OpType::Add, {lhsBase, rhsBase}, bitWidth).id;
    return ctx.replace(id, store->create(OpType::Mul, {diff, sum}, bitWidth).id);
}

Rule Get_DifferenceOfSquares_Rule() {
    return Rule{DifferenceOfSquares, &Match_DifferenceOfSquares, &Rewrite_DifferenceOfSquares, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
