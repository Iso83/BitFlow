#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_MulCombineConstants(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Mul || e.inputs.size() < 2)
        return false;

    int constCount = 0;
    for (const auto a : e.inputs) {
        const Expr& exprA = (*store)[a];

        if (exprA.op == OpType::Const)
            ++constCount;
        if (constCount >= 2)
            return true;
    }

    return false;
}

static ExprId Rewrite_MulCombineConstants(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(e.op == OpType::Mul);

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);
    Types::ExprChunk product = 1;
    int constCount = 0;
    ExprInputs nonConst;
    nonConst.reserve(e.inputs.size());

    for (auto a : e.inputs) {
        const Expr& exprA = (*store)[a];
        if (exprA.op == OpType::Const) {
            ++constCount;
            product = (product * static_cast<Types::ExprChunk>(exprA.knownValue)) & mask;
            continue;
        }

        nonConst.push_back(a);
    }

    if (constCount > 1) {
        const ExprId productId = store->createConstant(product, bitWidth).id;

        if (nonConst.empty())
            return ctx.replace(id, productId);

        nonConst.push_back(productId);

        if (nonConst.size() == 1)
            return ctx.replace(id, nonConst[0]);

        return ctx.replace(id, store->create(OpType::Mul, std::move(nonConst), bitWidth).id);
    }

    BF_CORE_ASSERT(false);
    return id;
}

Rule Get_MulCombineConstants_Rule() {
    return Rule{MulCombineConstants, &Match_MulCombineConstants, &Rewrite_MulCombineConstants, {AddCommonFactor}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
