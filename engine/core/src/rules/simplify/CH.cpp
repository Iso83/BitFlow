#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

static bool IsNotOf(const Expr* a, const Expr* b) {
    return a->op == OpType::Not && a->inputs.size() == 1 && a->inputs[0] == b;
}

struct SelectBranch {
    Expr* x;
    Expr* y;
};

static bool TryGetSelectBranch(Expr* e, SelectBranch& out) {
    // canonical CH branch: (x & y)
    if (e->op == OpType::And && e->inputs.size() == 2) {
        out.x = e->inputs[0];
        out.y = e->inputs[1];
        return true;
    }

    // collapsed form after child simplification: (x & x) -> x
    // treat as synthetic (x & x)
    out.x = e;
    out.y = e;
    return true;
}

static bool TryMatchCH(Expr* lhs, Expr* rhs, Expr*& x, Expr*& y, Expr*& z) {
    SelectBranch a{};
    if (!TryGetSelectBranch(lhs, a))
        return false;

    if (rhs->op != OpType::And || rhs->inputs.size() != 2)
        return false;

    Expr* r0 = rhs->inputs[0];
    Expr* r1 = rhs->inputs[1];

    if (IsNotOf(r0, a.x)) {
        x = a.x;
        y = a.y;
        z = r1;
        return true;
    }

    if (IsNotOf(r1, a.x)) {
        x = a.x;
        y = a.y;
        z = r0;
        return true;
    }

    return false;
}

static bool Match_CH(const Expr& e) {
    if (e.op != OpType::Xor || e.inputs.size() != 2)
        return false;

    Expr *x = nullptr, *y = nullptr, *z = nullptr;

    return TryMatchCH(e.inputs[0], e.inputs[1], x, y, z) || TryMatchCH(e.inputs[1], e.inputs[0], x, y, z);
}

static Expr* Rewrite_CH(Expr& e) {
    Expr *x = nullptr, *y = nullptr, *z = nullptr;

    if (!TryMatchCH(e.inputs[0], e.inputs[1], x, y, z) && !TryMatchCH(e.inputs[1], e.inputs[0], x, y, z))
        return nullptr;

    // CH(x, y, y) = y
    if (y == z)
        return y;

    // CH(x, x, z) = x | z
    if (x == y)
        return Expression::MakeOpInterned(OpType::Or, {x, z});

    // CH(x, y, x) = x & y
    if (x == z)
        return Expression::MakeOpInterned(OpType::And, {x, y});

    // CH(x, y, ~y) = ~(x ^ y)
    if (IsNotOf(z, y)) {
        Expr* xy = Expression::MakeOpInterned(OpType::Xor, {x, y});
        return Expression::MakeOpInterned(OpType::Not, {xy});
    }

    // default: reconstruct canonical CH
    Expr* nx = Expression::MakeOpInterned(OpType::Not, {x});
    Expr* a = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* b = Expression::MakeOpInterned(OpType::And, {nx, z});

    return Expression::MakeOpInterned(OpType::Xor, {a, b});
}

Rule Get_CH_Simplify_Rule() {
    return Rule{RuleId::Simplify_CH,
                &Match_CH,
                &Rewrite_CH,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify