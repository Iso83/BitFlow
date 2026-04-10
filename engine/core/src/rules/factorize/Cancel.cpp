#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize {

using Expr = AST::Expr;

#pragma region Match
static bool Match_Xor_Xor_CancelPair(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* l = e.inputs[0];
    Expr* r = e.inputs[1];

    if (l->op != AST::OpType::Xor || r->op != AST::OpType::Xor)
        return false;

    if (l->inputs.size() != 2 || r->inputs.size() != 2)
        return false;

    Expr* a0 = l->inputs[0];
    Expr* a1 = l->inputs[1];
    Expr* b0 = r->inputs[0];
    Expr* b1 = r->inputs[1];

    return (a0->id == b0->id) || (a0->id == b1->id) || (a1->id == b0->id) || (a1->id == b1->id);
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Xor_Xor_CancelPair(Expr& e) {
    Expr* l = e.inputs[0];
    Expr* r = e.inputs[1];

    Expr* a0 = l->inputs[0];
    Expr* a1 = l->inputs[1];
    Expr* b0 = r->inputs[0];
    Expr* b1 = r->inputs[1];

    Expr* otherL = nullptr;
    Expr* otherR = nullptr;

    if (a0->id == b0->id) {
        otherL = a1;
        otherR = b1;
    } else if (a0->id == b1->id) {
        otherL = a1;
        otherR = b0;
    } else if (a1->id == b0->id) {
        otherL = a0;
        otherR = b1;
    } else {
        otherL = a0;
        otherR = b0;
    }

    Expr* n = new Expr{};
    n->op = AST::OpType::Xor;
    n->inputs = {otherL, otherR};
    return n;
}
#pragma endregion

Rule Get_Xor_Pair_Cancel_Rule() {
    return Rule{RuleId::Factorize_XorPairCancel,
                &Match_Xor_Xor_CancelPair,
                &Rewrite_Xor_Xor_CancelPair,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Simplify_XorCancel}};
}

} // namespace BitFlow::Core::Rules::Factorize