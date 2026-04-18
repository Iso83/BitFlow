#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

static bool Match_CH(const Expr& e) {
    return e.op == OpType::Ch && e.inputs.size() == 3;
}

static Expr* Rewrite_CH(Expr& e) {
    Expr* x = e.inputs[0];
    Expr* y = e.inputs[1];
    Expr* z = e.inputs[2];

    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* nx = Expression::MakeOpInterned(OpType::Not, {x});
    Expr* nxz = Expression::MakeOpInterned(OpType::And, {nx, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, nxz});
}

Rule Get_CH_Simplify_Rule() {
    return Rule{RuleId::Simplify_CH,
                &Match_CH,
                &Rewrite_CH,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify
