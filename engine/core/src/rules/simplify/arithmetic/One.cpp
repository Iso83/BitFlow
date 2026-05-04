#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_Mul_One(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::Mul)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && exprIn.knownValue == 1)
            return true;
    }

    return false;
}

static bool Match_Div_One(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::Div)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& rhs = store->get(e.inputs[1]);
    return rhs.op == OpType::Const && rhs.knownValue == 1;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_Mul_One(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (!(exprIn.op == OpType::Const && exprIn.knownValue == 1))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->createConstant(1, e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Div_One(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    return e.inputs[0];
}
#pragma endregion

Rule Get_Mul_One_Rule() {
    return Rule{RuleId::Simplify_MulOne,     &Match_Mul_One,        &Rewrite_Mul_One, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::Arithmetic, "Simplify_MulOne"};
}

Rule Get_Div_One_Rule() {
    return Rule{RuleId::Simplify_DivOne,     &Match_Div_One,        &Rewrite_Div_One, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::Arithmetic, "Simplify_DivOne"};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
