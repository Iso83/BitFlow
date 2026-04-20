#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

static bool SameExpr(const Expr* a, const Expr* b) {
    return a && b && a->id.value() == b->id.value();
}

static bool Capture_CH_BitwiseEquivalent(const Expr& e, Expr*& x, Expr*& y, Expr*& z) {
    if (e.op != OpType::Xor || e.inputs.size() != 2)
        return false;

    for (size_t i = 0; i < 2; ++i) {
        Expr* maybeZ = e.inputs[i];
        Expr* maybeAnd = e.inputs[1 - i];
        if (maybeAnd->op != OpType::And || maybeAnd->inputs.size() != 2)
            continue;

        for (size_t j = 0; j < 2; ++j) {
            Expr* maybeX = maybeAnd->inputs[j];
            Expr* maybeXor = maybeAnd->inputs[1 - j];
            if (maybeXor->op != OpType::Xor || maybeXor->inputs.size() != 2)
                continue;

            Expr* lhs = maybeXor->inputs[0];
            Expr* rhs = maybeXor->inputs[1];
            if (SameExpr(lhs, maybeZ)) {
                x = maybeX;
                y = rhs;
                z = maybeZ;
                return true;
            }

            if (SameExpr(rhs, maybeZ)) {
                x = maybeX;
                y = lhs;
                z = maybeZ;
                return true;
            }
        }
    }

    return false;
}

static bool Match_CH(const Expr& e) {
    return e.op == OpType::Ch && e.inputs.size() == 3;
}

static Expr* MakeCanonical_CH(Expr* x, Expr* y, Expr* z) {
    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* nx = Expression::MakeOpInterned(OpType::Not, {x});
    Expr* nxz = Expression::MakeOpInterned(OpType::And, {nx, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, nxz});
}

static bool Match_CH_Canonicalize(const Expr& e) {
    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    return Capture_CH_BitwiseEquivalent(e, x, y, z);
}

static Expr* Rewrite_CH_Canonicalize(Expr& e) {
    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    if (!Capture_CH_BitwiseEquivalent(e, x, y, z))
        return nullptr;

    return MakeCanonical_CH(x, y, z);
}

static Expr* Rewrite_CH(Expr& e) {
    Expr* x = e.inputs[0];
    Expr* y = e.inputs[1];
    Expr* z = e.inputs[2];

    return MakeCanonical_CH(x, y, z);
}

Rule Get_CH_Canonicalize_Rule() {
    return Rule{RuleId::Simplify_CHCanonicalize,
                &Match_CH_Canonicalize,
                &Rewrite_CH_Canonicalize,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

Rule Get_CH_Simplify_Rule() {
    return Rule{RuleId::Simplify_CH,
                &Match_CH,
                &Rewrite_CH,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify
