#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using Expr = AST::Expr;
using OpType = AST::OpType;
using ConstPool = Expression::ConstPool;
using namespace BitFlow::Core::Expression;

static bool Match_Mul_CombineConstants(const Expr& e) {
    if (e.op != OpType::Mul || e.inputs.size() < 2)
        return false;

    int constCount = 0;
    for (const Expr* in : e.inputs) {
        if (in->isConst())
            ++constCount;
        if (constCount >= 2)
            return true;
    }

    return false;
}

static Expr* Rewrite_Mul_CombineConstants(Expr& e) {
    uint64_t product = 1;
    int constCount = 0;
    std::vector<Expr*> nonConst;
    nonConst.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (in->isConst()) {
            ++constCount;
            product *= static_cast<uint64_t>(in->constValue);
            continue;
        }

        nonConst.push_back(in);
    }

    if (constCount < 2)
        return nullptr;

    std::vector<Expr*> merged;
    merged.reserve(nonConst.size() + 1);
    merged.push_back(ConstPool::Get(static_cast<uint32_t>(product)));
    for (Expr* in : nonConst)
        merged.push_back(in);

    return MakeOpInterned(OpType::Mul, merged);
}

Rule Get_Mul_CombineConstants_Rule() {
    return Rule{RuleId::Factorize_MulCombineConstants, &Match_Mul_CombineConstants, &Rewrite_Mul_CombineConstants,
                Stage_Factorize,
                {RuleId::Factorize_AddLinearMultiplicity, RuleId::Factorize_AddCommonFactor, RuleId::Normalize_Flatten,
                 RuleId::Normalize_Order},
                RuleFlags::Factorizing | RuleFlags::Arithmetic, "Factorize_MulCombineConstants"};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
