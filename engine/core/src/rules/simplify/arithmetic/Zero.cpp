#include "expression/ExprClone.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using Expr = AST::Expr;

static Expr* Rewrite_Add_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!(in->isConst() && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);

    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Mul_Zero(Expr&) {
    return Expression::ConstPool::Get(0);
}

static bool Match_Sub_Zero(const Expr& e) {
    if (e.op != AST::OpType::Sub)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    return rhs->isConst() && rhs->constValue == 0;
}

static Expr* Rewrite_Sub_Zero(Expr& e) {
    return e.inputs[0];
}

static bool Match_Mod_Zero(const Expr& e) {
    if (e.op != AST::OpType::Mod)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    return rhs->isConst() && rhs->constValue == 0;
}

static Expr* Rewrite_Mod_Zero_Guard(Expr&) {
    // Keep `% 0` explicit in the AST for now.
    // We intentionally do not fold or rewrite invalid modulo forms.
    return nullptr;
}

static bool Match_Shift_Zero(const Expr& e) {
    switch (e.op) {
    case AST::OpType::Shl:
    case AST::OpType::Shr:
    case AST::OpType::UShr:
        break;
    default:
        return false;
    }

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    return rhs->isConst() && rhs->constValue == 0;
}

static Expr* Rewrite_Shift_Zero(Expr& e) {
    return e.inputs[0];
}

static bool Match_Rotate_Modulo_Bitwidth(const Expr& e) {
    switch (e.op) {
    case AST::OpType::RotL:
    case AST::OpType::RotR:
        break;
    default:
        return false;
    }

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    if (!rhs->isConst())
        return false;

    constexpr uint32_t kBitWidth = 32;
    const uint32_t amount = rhs->constValue;

    return amount == 0 || amount >= kBitWidth;
}

static Expr* Rewrite_Rotate_Modulo_Bitwidth(Expr& e) {
    constexpr uint32_t kBitWidth = 32;
    const uint32_t amount = e.inputs[1]->constValue % kBitWidth;

    if (amount == 0)
        return e.inputs[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs[1] = Expression::ConstPool::Get(amount);
    return target;
}

Rule Get_Add_Zero_Rule() {
    return Rule{RuleId::Simplify_AddZero,
                &Match_Zero<AST::OpType::Add>,
                &Rewrite_Add_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Mul_Zero_Rule() {
    return Rule{RuleId::Simplify_MulZero,
                &Match_Zero<AST::OpType::Mul>,
                &Rewrite_Mul_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Sub_Zero_Rule() {
    return Rule{
        RuleId::Simplify_SubZero, &Match_Sub_Zero, &Rewrite_Sub_Zero, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

Rule Get_Mod_Zero_Guard_Rule() {
    return Rule{RuleId::Simplify_ModZeroGuard,
                &Match_Mod_Zero,
                &Rewrite_Mod_Zero_Guard,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Shift_Zero_Rule() {
    return Rule{RuleId::Simplify_ShiftZero,
                &Match_Shift_Zero,
                &Rewrite_Shift_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Rotate_Modulo_Bitwidth_Rule() {
    return Rule{RuleId::Simplify_RotateModuloBitwidth,
                &Match_Rotate_Modulo_Bitwidth,
                &Rewrite_Rotate_Modulo_Bitwidth,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
