#include "expression/ExprUtils.h"

#include <BitFlow/core/helper/Attributes.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
BF_DEPRECATED("Use Match_Zero --> not limited to 2 leafs")
static bool Match_Sub_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Sub)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return IsFalse(store, e.inputs[1]);
}

BF_DEPRECATED("Use Match_Zero --> not limited to 2 leafs")
static bool Match_Mod_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Mod)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return IsFalse(store, e.inputs[1]);
}

BF_DEPRECATED("TODO: fix limited to 2 leafs")
static bool Match_Shift_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    switch (e.op) {
    case OpType::Shl:
    case OpType::Shr:
        break;
    default:
        return false;
    }

    if (e.inputs.size() != 2)
        return false;

    return IsFalse(store, e.inputs[1]);
}

BF_DEPRECATED("TODO: fix limited to 2 leafs")
static bool Match_Rotate_Zero(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    switch (e.op) {
    case OpType::RotL:
    case OpType::RotR:
        break;
    default:
        return false;
    }

    if (e.inputs.size() != 2)
        return false;

    return IsFalse(store, e.inputs[1]);
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_Add_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (!(exprIn.op == OpType::Const && IsFalse(store, in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Mul_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_Sub_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    return e.inputs[0];
}

static ExprId Rewrite_Mod_Zero_Guard(ExprStore* store, ExprId id) {
    const auto& e = store->get(id);

    for (auto input : e.inputs) {
        const auto& rhs = store->get(input);
        if (rhs.op == OpType::Const && rhs.knownValue == 0) {
            throw std::runtime_error("Modulo by zero detected in rewrite");
        }
    }

    return id;
}

static ExprId Rewrite_Shift_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    return e.inputs[0];
}

static ExprId Rewrite_Rotate_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
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
    return Rule{Mod_Zero_Guard, &Match_Mod_Zero, &Rewrite_Mod_Zero_Guard, {Normalize::Flatten}};
}

Rule Get_Shift_Zero_Rule() {
    return Rule{Shift_Zero, &Match_Shift_Zero, &Rewrite_Shift_Zero, {Normalize::Flatten}};
}

Rule Get_Rotate_Zero_Rule() {
    return Rule{Rotate_Zero, &Match_Rotate_Zero, &Rewrite_Rotate_Zero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
