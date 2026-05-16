#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_AddFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Add)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const)
            constCount++;
    }

    return constCount >= 2;
}

static bool Match_SubConstFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Sub || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    if (lhs.op != OpType::Add)
        return false;

    if (lhs.inputs.size() < 2)
        return false;

    if (rhs.op != OpType::Const)
        return false;

    int constCount = 0;

    for (ExprId in : lhs.inputs) {
        if ((*store)[in].op == OpType::Const)
            constCount++;
    }

    return constCount >= 1;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddFold(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            hasConst = true;
        } else {
            nonConst.push_back(inId);
        }
    }

    if (!hasConst)
        return id;

    const Types::BitWidth bitWidth = e.bitWidth;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Add, std::move(nonConst), bitWidth).id;
}

static ExprId Rewrite_SubConstFold(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk acc = 0;

    std::vector<ExprId> newInputs;
    newInputs.reserve(lhs.inputs.size());

    bool changed = false;

    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            changed = true;
        } else
            newInputs.push_back(inId);
    }

    if (!changed)
        return id;

    acc = (acc - rhs.knownValue) & mask;

    if (acc != 0)
        newInputs.push_back(store->createConstant(acc, bitWidth).id);

    if (newInputs.empty())
        return store->createConstant(0, bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Add, std::move(newInputs), bitWidth).id;
}
#pragma endregion

Rule Get_AddFold_Rule() {
    return Rule{AddFold, &Match_AddFold, &Rewrite_AddFold, {Normalize::Flatten}};
}

Rule Get_SubConstFold_Rule() {
    return Rule{SubConstFold, &Match_SubConstFold, &Rewrite_SubConstFold, {Simplify::Arithmetic::AddFold}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
