#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = Expression::ExprOld;
using OpType = Expression::OpType;

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

static Expr* MakeCanonical_CH(Expr* x, Expr* y, Expr* z) {
    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* nx = Expression::MakeOpInterned(OpType::Not, {x});
    Expr* nxz = Expression::MakeOpInterned(OpType::And, {nx, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, nxz});
}

static bool Match_CH(const Expr& e) {
    if (e.op == OpType::Ch && e.inputs.size() == 3)
        return true;

    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    return Capture_CH_BitwiseEquivalent(e, x, y, z);
}

static Expr* Rewrite_CH(Expr& e) {
    if (e.op == OpType::Ch && e.inputs.size() == 3)
        return MakeCanonical_CH(e.inputs[0], e.inputs[1], e.inputs[2]);

    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    if (!Capture_CH_BitwiseEquivalent(e, x, y, z))
        return nullptr;

    return MakeCanonical_CH(x, y, z);
}

Rule Get_CH_Simplify_Rule() {
    return Rule{RuleId::Simplify_CH,
                &Match_CH,
                &Rewrite_CH,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::None,
                "Simplify_CH"};
}

} // namespace BitFlow::Core::Rules::Simplify
