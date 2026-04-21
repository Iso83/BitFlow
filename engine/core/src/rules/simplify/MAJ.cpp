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

static bool IsAnd2(const Expr* e) {
    return e && e->op == OpType::And && e->inputs.size() == 2;
}

static bool Capture_MAJ_BitwiseEquivalent(const Expr& e, Expr*& x, Expr*& y, Expr*& z) {
    if (e.op != OpType::Or || e.inputs.size() != 2)
        return false;

    for (size_t i = 0; i < 2; ++i) {
        Expr* maybeXY = e.inputs[i];
        Expr* maybeZXor = e.inputs[1 - i];

        if (!IsAnd2(maybeXY) || !IsAnd2(maybeZXor))
            continue;

        Expr* candidateX = maybeXY->inputs[0];
        Expr* candidateY = maybeXY->inputs[1];

        for (size_t j = 0; j < 2; ++j) {
            Expr* maybeZ = maybeZXor->inputs[j];
            Expr* maybeXor = maybeZXor->inputs[1 - j];
            if (maybeXor->op != OpType::Xor || maybeXor->inputs.size() != 2)
                continue;

            Expr* lhs = maybeXor->inputs[0];
            Expr* rhs = maybeXor->inputs[1];
            const bool samePair = (SameExpr(lhs, candidateX) && SameExpr(rhs, candidateY)) ||
                                  (SameExpr(lhs, candidateY) && SameExpr(rhs, candidateX));
            if (!samePair)
                continue;

            x = candidateX;
            y = candidateY;
            z = maybeZ;
            return true;
        }
    }

    return false;
}

static Expr* MakeCanonical_MAJ(Expr* x, Expr* y, Expr* z) {
    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* xz = Expression::MakeOpInterned(OpType::And, {x, z});
    Expr* yz = Expression::MakeOpInterned(OpType::And, {y, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, xz, yz});
}

static bool Match_MAJ(const Expr& e) {
    if (e.op == OpType::Maj && e.inputs.size() == 3)
        return true;

    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    return Capture_MAJ_BitwiseEquivalent(e, x, y, z);
}

static Expr* Rewrite_MAJ(Expr& e) {
    if (e.op == OpType::Maj && e.inputs.size() == 3)
        return MakeCanonical_MAJ(e.inputs[0], e.inputs[1], e.inputs[2]);

    Expr* x = nullptr;
    Expr* y = nullptr;
    Expr* z = nullptr;
    if (!Capture_MAJ_BitwiseEquivalent(e, x, y, z))
        return nullptr;

    return MakeCanonical_MAJ(x, y, z);
}

Rule Get_MAJ_Simplify_Rule() {
    return Rule{RuleId::Simplify_MAJ,
                &Match_MAJ,
                &Rewrite_MAJ,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::None,
                "Simplify_MAJ"};
}

} // namespace BitFlow::Core::Rules::Simplify
