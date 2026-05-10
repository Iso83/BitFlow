#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <stdexcept>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool IsConstFalse(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Const && e.inputs.empty() && store->isFalse(id);
}

#pragma region Matching
template <OpType Op> static bool Match_RightZero(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != Op)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (IsConstFalse(store, e.inputs[i]))
            return true;
    }

    return false;
}

static bool Match_Sub_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Sub)
        return false;

    if (e.inputs.size() < 2)
        return false;

    if (IsConstFalse(store, e.inputs[0]) && e.inputs.size() == 2)
        return true;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (IsConstFalse(store, e.inputs[i]))
            return true;
    }

    return false;
}

static bool Match_Shift_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Shl && e.op != OpType::Shr)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return IsConstFalse(store, e.inputs[1]);
}

static bool Match_Rotate_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::RotL && e.op != OpType::RotR)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return IsConstFalse(store, e.inputs[1]);
}
#pragma endregion

#pragma region Rewrite

static ExprId Rewrite_Add_Zero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    std::vector<ExprId> newInputs;
    newInputs.reserve(inputs.size());

    for (auto in : inputs) {
        if (IsConstFalse(store, in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Add, std::move(newInputs), bitWidth).id;
}

static ExprId Rewrite_Mul_Zero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_Sub_Zero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    if (inputs.size() == 2 && IsConstFalse(store, inputs[0]))
        return store->create(OpType::Neg, {inputs[1]}, bitWidth).id;

    std::vector<ExprId> newInputs;
    newInputs.reserve(inputs.size());

    newInputs.push_back(inputs[0]);

    for (size_t i = 1; i < inputs.size(); ++i) {
        if (IsConstFalse(store, inputs[i]))
            continue;

        newInputs.push_back(inputs[i]);
    }

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Sub, std::move(newInputs), bitWidth).id;
}

static ExprId Rewrite_Mod_Zero_Guard(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;

    for (size_t i = 1; i < inputs.size(); ++i) {
        if (IsConstFalse(store, inputs[i]))
            throw std::runtime_error("Modulo by zero detected in rewrite");
    }

    return id;
}

static ExprId Rewrite_Shift_Zero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return e.inputs[0];
}

static ExprId Rewrite_Rotate_Zero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return e.inputs[0];
}
#pragma endregion

Rule Get_Add_Zero_Rule() {
    return Rule{Add_Zero, &Match_Zero<OpType::Add>, &Rewrite_Add_Zero, {Normalize::Flatten}};
}

Rule Get_Mul_Zero_Rule() {
    return Rule{Mul_Zero, &Match_Zero<OpType::Mul>, &Rewrite_Mul_Zero, {Normalize::Flatten}};
}

Rule Get_Sub_Zero_Rule() {
    return Rule{Sub_Zero, &Match_Sub_Zero, &Rewrite_Sub_Zero, {Normalize::Flatten}};
}

Rule Get_Mod_Zero_Guard_Rule() {
    return Rule{Mod_Zero_Guard, &Match_RightZero<OpType::Mod>, &Rewrite_Mod_Zero_Guard, {Normalize::Flatten}};
}

Rule Get_Shift_Zero_Rule() {
    return Rule{Shift_Zero, &Match_Shift_Zero, &Rewrite_Shift_Zero, {Normalize::Flatten}};
}

Rule Get_Rotate_Zero_Rule() {
    return Rule{Rotate_Zero, &Match_Rotate_Zero, &Rewrite_Rotate_Zero, {Normalize::Bitwise::Rotate_ModuloBitWidth}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic