#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Mul_CombineConstants(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Mul || e.inputs.size() < 2)
        return false;

    int constCount = 0;
    for (const auto a : e.inputs) {
        const Expr& exprA = store->get(a);

        if (exprA.op == OpType::Const)
            ++constCount;
        if (constCount >= 2)
            return true;
    }

    return false;
}

static ExprId Rewrite_Mul_CombineConstants(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    _ASSERT(e.op == OpType::Mul);

    uint64_t product = 1;
    int constCount = 0;
    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    for (auto a : e.inputs) {
        const Expr& exprA = store->get(a);
        if (exprA.op == OpType::Const) {
            ++constCount;
            product *= static_cast<uint64_t>(exprA.knownValue);
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

Rule Get_Mul_CombineConstants_Rule() {
    return Rule{RuleId::Factorize_MulCombineConstants,
                &Match_Mul_CombineConstants,
                &Rewrite_Mul_CombineConstants,
                Stage_Factorize,
                {RuleId::Factorize_AddLinearMultiplicity, RuleId::Factorize_AddCommonFactor, RuleId::Normalize_Flatten,
                 RuleId::Normalize_Order},
                RuleFlags::Factorizing | RuleFlags::Arithmetic,
                "Factorize_MulCombineConstants"};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
