#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Expression;

static Expr* Rewrite_Remove_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if ((in->op != OpType::Const && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);

    target->inputs = std::move(newInputs);
    return target;
}

Rule Get_Xor_Zero_Rule() {
    return Rule{RuleId::Simplify_XorZero,    &Match_Zero<OpType::Xor>, &Rewrite_Remove_Zero, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None,          "Simplify_XorZero"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
