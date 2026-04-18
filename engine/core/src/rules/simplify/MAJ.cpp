#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

static bool Match_MAJ(const Expr& e) {
    return e.op == OpType::Maj && e.inputs.size() == 3;
}

static Expr* Rewrite_MAJ(Expr& e) {
    Expr* x = e.inputs[0];
    Expr* y = e.inputs[1];
    Expr* z = e.inputs[2];

    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* xz = Expression::MakeOpInterned(OpType::And, {x, z});
    Expr* yz = Expression::MakeOpInterned(OpType::And, {y, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, xz, yz});
}

Rule Get_MAJ_Simplify_Rule() {
    return Rule{RuleId::Simplify_MAJ,
                &Match_MAJ,
                &Rewrite_MAJ,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify
