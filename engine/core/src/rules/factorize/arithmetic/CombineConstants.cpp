#include "expression/ExprUtils.h"

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

static ExprId Rewrite_MulCombineConstants(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    _ASSERT(e.op == OpType::Mul);

    Types::ExprChunk product = 1;
    int constCount = 0;
    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    for (auto a : e.inputs) {
        const Expr& exprA = (*store)[a];
        if (exprA.op == OpType::Const) {
            ++constCount;
            product *= static_cast<Types::ExprChunk>(exprA.knownValue);
            continue;
        }

        nonConst.push_back(a);
    }

    if (constCount > 1) {
        nonConst.push_back(store->createConstant(product, e.bitWidth).id);
        return store->create(OpType::Mul, std::move(nonConst), e.bitWidth).id;
    }

    _ASSERT(false);
    return id;
}

Rule Get_MulCombineConstants_Rule() {
    return Rule{MulCombineConstants, &Match_MulCombineConstants, &Rewrite_MulCombineConstants, {AddCommonFactor}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
