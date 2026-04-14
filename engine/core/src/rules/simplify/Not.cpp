#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

static bool Match_Not_Xor(const Expr& e) {
    if (e.op != AST::OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr* in = e.inputs[0];

    return (in->op == AST::OpType::Xor && in->inputs.size() >= 1);
}

static Expr* Rewrite_Not_Xor(Expr& e) {
    Expr* in = e.inputs[0];

    std::vector<Expr*> newInputs;
    newInputs.reserve(in->inputs.size() + 1);

    for (Expr* child : in->inputs)
        newInputs.push_back(child);

    newInputs.push_back(Expression::ConstPool::Get(1));

    Expr* target = Expression::MakeOpInterned(AST::OpType::Xor, std::move(newInputs));
    return target;
}

Rule Get_Not_Xor_Rule() {
    return Rule{RuleId::Simplify_NotXor, &Match_Not_Xor, &Rewrite_Not_Xor, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify
