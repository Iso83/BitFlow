#include "expression/ExprUtils.h"
#include "rules/RuleDiagnostics.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <stdexcept>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
template <OpType Op> static bool Match_RightZero(const ExprStore* store, const ExprNameMap* names, ExprId id) {
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
#pragma endregion

static bool Match_SubSelf(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Sub)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return e.inputs[0] == e.inputs[1];
}

static bool Match_ModSelf(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Mod)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return e.inputs[0] == e.inputs[1];
}

static bool Match_PowZero(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Pow)
        return false;

    if (e.inputs.size() != 2)
        return false;

    return IsConstFalse(store, e.inputs[1]);
}

#pragma region Rewrite
static ExprId Rewrite_AddZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    ExprInputs newInputs;
    newInputs.reserve(inputs.size());

    for (auto in : inputs) {
        if (IsConstFalse(store, in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->makeFalse(bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(OpType::Add, std::move(newInputs), bitWidth).id);
}

static ExprId Rewrite_MulZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    if (e.bitWidth == 0)
        BF_RULE_ERROR("Rewrite_MulZero encountered invalid bitWidth 0");

    return ctx.replace(id, store->makeFalse(e.bitWidth).id);
}

static ExprId Rewrite_SubZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    if (inputs.size() == 2 && IsConstFalse(store, inputs[0]))
        return ctx.replace(id, store->create(OpType::Neg, {inputs[1]}, bitWidth).id);

    ExprInputs newInputs;
    newInputs.reserve(inputs.size());

    newInputs.push_back(inputs[0]);

    for (size_t i = 1; i < inputs.size(); ++i) {
        if (IsConstFalse(store, inputs[i]))
            continue;

        newInputs.push_back(inputs[i]);
    }

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(OpType::Sub, std::move(newInputs), bitWidth).id);
}

static ExprId Rewrite_SubSelf(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return ctx.replace(id, store->makeFalse(e.bitWidth).id);
}

static ExprId Rewrite_ModSelf(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return ctx.replace(id, store->makeFalse(e.bitWidth).id);
}

static ExprId Rewrite_PowZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return ctx.replace(id, store->createConstant(1, e.bitWidth).id);
}

static ExprId Rewrite_ShiftZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(store->isFalse(e.inputs[1]));

    return ctx.replace(id, e.inputs[0]);
}

static ExprId Rewrite_RotateZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(store->isFalse(e.inputs[1]));

    return ctx.replace(id, e.inputs[0]);
}
#pragma endregion

Rule Get_AddZero_Rule() {
    return Rule{AddZero, &Match_Zero<OpType::Add>, &Rewrite_AddZero, {Normalize::Order}};
}

Rule Get_MulZero_Rule() {
    return Rule{MulZero, &Match_Zero<OpType::Mul>, &Rewrite_MulZero, {Normalize::Flatten}};
}

Rule Get_SubZero_Rule() {
    return Rule{SubZero, &Match_Zero<OpType::Sub>, &Rewrite_SubZero, {Normalize::Flatten}};
}

Rule Get_SubSelf_Rule() {
    return Rule{SubSelf, &Match_SubSelf, &Rewrite_SubSelf, {Normalize::Flatten}};
}

Rule Get_ModSelf_Rule() {
    return Rule{ModSelf, &Match_ModSelf, &Rewrite_ModSelf, {Normalize::Flatten}};
}

Rule Get_PowZero_Rule() {
    return Rule{PowZero, &Match_PowZero, &Rewrite_PowZero, {Normalize::Flatten}};
}

Rule Get_ShiftZero_Rule() {
    return Rule{ShiftZero,
                [](const ExprStore* store, const ExprNameMap* names, ExprId id) {
                    return Match_RightZero<OpType::Shl>(store, names, id) ||
                           Match_RightZero<OpType::Shr>(store, names, id);
                },
                &Rewrite_ShiftZero,
                {Normalize::Flatten}};
}

Rule Get_RotateZero_Rule() {
    return Rule{RotateZero,
                [](const ExprStore* store, const ExprNameMap* names, ExprId id) {
                    return Match_RightZero<OpType::RotL>(store, names, id) ||
                           Match_RightZero<OpType::RotR>(store, names, id);
                },
                &Rewrite_RotateZero,
                {Normalize::Bitwise::RotateModulo}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
