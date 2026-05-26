#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_AndFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && (store->isFalse(in) || store->isTrue(in)))
            return true;
    }

    return false;
}

static bool Match_OrFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && (store->isTrue(in) || store->isFalse(in)))
            return true;
    }

    return false;
}

static bool Match_XorFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Xor)
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
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AndFold(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    ExprInputs newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            return store->makeFalse(e.bitWidth).id;

        if (exprIn.op == OpType::Const && store->isTrue(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeTrue(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_OrFold(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    ExprInputs newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isTrue(in))
            return store->makeTrue(e.bitWidth).id;

        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_XorFold(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    ExprInputs nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc ^= in.knownValue;
            hasConst = true;
        } else {
            nonConst.push_back(inId);
        }
    }

    acc &= mask;

    if (!hasConst)
        return id;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Xor, std::move(nonConst), e.bitWidth).id;
}
#pragma endregion

Rule Get_AndFold_Rule() {
    return Rule{AndFold, &Match_AndFold, &Rewrite_AndFold, {Normalize::Flatten}};
}

Rule Get_OrFold_Rule() {
    return Rule{OrFold, &Match_OrFold, &Rewrite_OrFold, {Normalize::Flatten}};
}

Rule Get_XorFold_Rule() {
    return Rule{XorFold, &Match_XorFold, &Rewrite_XorFold, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
