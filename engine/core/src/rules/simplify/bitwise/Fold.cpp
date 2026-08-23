#include "expression/ExprUtils.h"

#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Engine::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

#pragma region Match
static bool Match_AndFold(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    size_t constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Const)
            continue;

        if (store->isFalse(in) || store->isTrue(in))
            return true;

        constCount++;
    }

    return constCount >= 2;
}

static bool Match_OrFold(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    size_t constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Const)
            continue;

        if (store->isTrue(in) || store->isFalse(in))
            return true;

        constCount++;
    }

    return constCount >= 2;
}

static bool Match_XorFold(const ExprStore* store, const ExprNameMap* names, ExprId id) {
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
static ExprId Rewrite_AndFold(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    BF_CORE_ASSERT(e.op == OpType::And);

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk acc = mask;
    bool hasConst = false;

    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const) {
            acc &= exprIn.knownValue;
            hasConst = true;

            if ((acc & mask) == 0)
                return ctx.replace(id, store->makeFalse(bitWidth).id);

            continue;
        }

        newInputs.push_back(in);
    }

    if (!hasConst)
        return id;

    acc &= mask;

    if (acc != mask)
        newInputs.insert(newInputs.begin(), store->createConstant(acc, bitWidth).id);

    if (newInputs.empty())
        return ctx.replace(id, store->makeTrue(bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(OpType::And, std::move(newInputs), bitWidth).id);
}

static ExprId Rewrite_OrFold(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    BF_CORE_ASSERT(e.op == OpType::Or);

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);
    Types::ExprChunk acc = 0;
    bool hasConst = false;

    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const) {
            acc |= exprIn.knownValue;
            hasConst = true;

            if ((acc & mask) == mask)
                return ctx.replace(id, store->makeTrue(bitWidth).id);

            continue;
        }

        newInputs.push_back(in);
    }

    if (!hasConst)
        return id;

    acc &= mask;

    if (acc != 0)
        newInputs.insert(newInputs.begin(), store->createConstant(acc, bitWidth).id);

    if (newInputs.empty())
        return ctx.replace(id, store->makeFalse(bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(OpType::Or, std::move(newInputs), bitWidth).id);
}

static ExprId Rewrite_XorFold(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    ExprInputs nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

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
        nonConst.push_back(store->createConstant(acc, bitWidth).id);

    if (nonConst.empty())
        return ctx.replace(id, store->zeroId());

    if (nonConst.size() == 1)
        return ctx.replace(id, nonConst[0]);

    return ctx.replace(id, store->create(OpType::Xor, std::move(nonConst), bitWidth).id);
}
#pragma endregion

Rule Get_AndFold_Rule() {
    return Rule{AndFold, &Match_AndFold, &Rewrite_AndFold, {Normalize::Order}};
}

Rule Get_OrFold_Rule() {
    return Rule{OrFold, &Match_OrFold, &Rewrite_OrFold, {Normalize::Order, Bitwise::Idempotent}};
}

Rule Get_XorFold_Rule() {
    return Rule{XorFold, &Match_XorFold, &Rewrite_XorFold, {Normalize::Order}};
}

} // namespace BitFlow::Engine::Core::Rules::Simplify::Bitwise
