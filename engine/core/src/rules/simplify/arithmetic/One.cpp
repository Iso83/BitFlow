#include "expression/ExprUtils.h"

#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Engine::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

#pragma region Match
static bool Match_MulOne(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Mul)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && exprIn.knownValue == 1)
            return true;
    }

    return false;
}

static bool Match_DivOne(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Div)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    return rhs.op == OpType::Const && rhs.knownValue == 1;
}

static bool Match_PowOne(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Pow)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    return rhs.op == OpType::Const && rhs.knownValue == 1;
}

static bool Match_DivSelf(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Div)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return e.inputs[0] == e.inputs[1];
}

static bool Match_ModOne(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Mod)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    return rhs.op == OpType::Const && rhs.knownValue == 1;
}

#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_MulOne(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (!(exprIn.op == OpType::Const && exprIn.knownValue == 1))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->createConstant(1, e.bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}

static ExprId Rewrite_DivOne(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    return ctx.replace(id, e.inputs[0]);
}

static ExprId Rewrite_PowOne(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    return ctx.replace(id, e.inputs[0]);
}

static ExprId Rewrite_DivSelf(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    return ctx.replace(id, store->createConstant(1, e.bitWidth).id);
}

static ExprId Rewrite_ModOne(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    return ctx.replace(id, store->zeroId());
}

#pragma endregion

Rule Get_MulOne_Rule() {
    return Rule{MulOne, &Match_MulOne, &Rewrite_MulOne, {Normalize::Order}};
}

Rule Get_DivOne_Rule() {
    return Rule{DivOne, &Match_DivOne, &Rewrite_DivOne, {Normalize::Flatten}};
}

Rule Get_PowOne_Rule() {
    return Rule{PowOne, &Match_PowOne, &Rewrite_PowOne, {Normalize::Flatten}};
}

Rule Get_DivSelf_Rule() {
    return Rule{DivSelf, &Match_DivSelf, &Rewrite_DivSelf, {Normalize::Flatten}};
}

Rule Get_ModOne_Rule() {
    return Rule{ModOne, &Match_ModOne, &Rewrite_ModOne, {Normalize::Flatten}};
}

} // namespace BitFlow::Engine::Core::Rules::Simplify::Arithmetic
