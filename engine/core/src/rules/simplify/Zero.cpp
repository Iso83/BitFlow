#include "expression/ExprClone.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

static Expr* Rewrite_Remove_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!(in->isConst && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;

    target->inputs = std::move(newInputs);
    return target;
}

Rule Get_Add_Zero_Rule() {
    return Rule{&Match_Zero<AST::OpType::Add>, &Rewrite_Remove_Zero, Stage_Simplify};
}

Rule Get_Xor_Zero_Rule() {
    return Rule{&Match_Zero<AST::OpType::Xor>, &Rewrite_Remove_Zero, Stage_Simplify};
}

} // namespace BitFlow::Core::Rules::Simplify