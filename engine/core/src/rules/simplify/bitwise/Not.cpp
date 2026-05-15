#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_Not(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = (*store)[e.inputs[0]];

    if (in.op == OpType::Not && in.inputs.size() == 1)
        return true;

    if (in.op == OpType::Const)
        return true;

    return false;
}

static bool Match_NotPushdown(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = (*store)[e.inputs[0]];

    if (!(in.op == OpType::And || in.op == OpType::Or))
        return false;

    bool allNot = true;
    for (auto child : in.inputs) {
        const Expr& exprChild = (*store)[child];
        if (exprChild.op != OpType::Not) {
            allNot = false;
            break;
        }
    }

    return !allNot;
}

static bool Match_NotXor(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = (*store)[e.inputs[0]];

    return (in.op == OpType::Xor && in.inputs.size() >= 1);
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_Not(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    ExprId in = e.inputs[0];
    const Expr& exprIn = (*store)[in];

    if (exprIn.op == OpType::Not && exprIn.inputs.size() == 1)
        return exprIn.inputs[0];

    if (exprIn.op == OpType::Const)
        return store->invertConst(in).id;

    _ASSERT(false);
    return id;
}

static ExprId Rewrite_NotPushdown(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    ExprId in = e.inputs[0];
    const Expr& exprIn = (*store)[in];

    OpType newOp = (exprIn.op == OpType::And) ? OpType::Or : OpType::And;

    const std::vector<ExprId> inputs = exprIn.inputs;
    std::vector<ExprId> newInputs;
    newInputs.reserve(inputs.size());

    for (auto child : inputs)
        newInputs.push_back(store->create(OpType::Not, {child}, bitWidth).id);

    return store->create(newOp, std::move(newInputs), bitWidth).id;
}

static ExprId Rewrite_NotXor(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    ExprId in = e.inputs[0];
    const Expr& exprIn = (*store)[in];

    std::vector<ExprId> newInputs;
    newInputs.reserve(exprIn.inputs.size() + 1);

    for (auto child : exprIn.inputs)
        newInputs.push_back(child);

    const Types::BitWidth bitWidth = e.bitWidth;

    newInputs.push_back(store->makeTrue(bitWidth).id);

    return store->create(OpType::Xor, std::move(newInputs), bitWidth).id;
}
#pragma endregion

Rule Get_Not_Rule() {
    return Rule{Not, &Match_Not, &Rewrite_Not};
}

Rule Get_NotPushdown_Rule() {
    return Rule{NotPushdown, &Match_NotPushdown, &Rewrite_NotPushdown};
}

Rule Get_NotXor_Rule() {
    return Rule{NotXor, &Match_NotXor, &Rewrite_NotXor, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
