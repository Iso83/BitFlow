#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Match
static bool Match_Xor_And_CommonFactor(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* left = e.inputs[0];
    Expr* right = e.inputs[1];

    if (left->op != AST::OpType::And || right->op != AST::OpType::And)
        return false;

    if (left->inputs.size() != 2 || right->inputs.size() != 2)
        return false;

    Expr* a0 = left->inputs[0];
    Expr* a1 = left->inputs[1];
    Expr* b0 = right->inputs[0];
    Expr* b1 = right->inputs[1];

    return (a0->id == b0->id) || (a0->id == b1->id) || (a1->id == b0->id) || (a1->id == b1->id);
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Xor_And_CommonFactor(Expr& e) {
    Expr* left = e.inputs[0];
    Expr* right = e.inputs[1];

    Expr* a0 = left->inputs[0];
    Expr* a1 = left->inputs[1];
    Expr* b0 = right->inputs[0];
    Expr* b1 = right->inputs[1];

    Expr* common = nullptr;
    Expr* otherLeft = nullptr;
    Expr* otherRight = nullptr;

    if (a0->id == b0->id) {
        common = a0;
        otherLeft = a1;
        otherRight = b1;
    } else if (a0->id == b1->id) {
        common = a0;
        otherLeft = a1;
        otherRight = b0;
    } else if (a1->id == b0->id) {
        common = a1;
        otherLeft = a0;
        otherRight = b1;
    } else {
        common = a1;
        otherLeft = a0;
        otherRight = b0;
    }

    Expr* innerXor = new Expr{};
    innerXor->op = OpType::Xor;
    innerXor->inputs = {otherLeft, otherRight};

    Expr* result = new Expr{};
    result->op = OpType::And;
    result->inputs = {common, innerXor};

    return result;
}
#pragma endregion

Rule Get_Xor_And_Rule() {
    return Rule{RuleId::Factorize_XorAnd,
                &Match_Xor_And_CommonFactor,
                &Rewrite_Xor_And_CommonFactor,
                Stage_Factorize,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize