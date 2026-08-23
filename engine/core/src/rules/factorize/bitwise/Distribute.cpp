#include "expression/ExprUtils.h"

#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Engine::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

#pragma region Helpers
static ExprId BuildAnd(ExprStore* store, ExprId a, ExprId b) {
    const Expr& exprA = (*store)[a];
    if (exprA.op == OpType::Const && store->isFalse(a))
        return a;

    const Expr& exprB = (*store)[b];
    if (exprB.op == OpType::Const && store->isFalse(b))
        return b;

    if (exprA.op == OpType::Const && store->isTrue(a))
        return b;

    if (exprB.op == OpType::Const && store->isTrue(b))
        return a;

    return store->create(OpType::And, {a, b}).id;
}
#pragma endregion

static bool Match_Distribute_And_Over_Xor(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Xor || exprIn.inputs.size() < 2)
            continue;

        const size_t termCount = exprIn.inputs.size();
        const size_t otherCount = e.inputs.size() - 1;

        if (termCount * (otherCount + 1) > 12)
            continue;

        return true;
    }

    return false;
}

static ExprId Rewrite_Distribute_And_Over_Xor(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    ExprId xorNodeID{};
    bool foundXor = false;

    for (auto in : inputs) {
        const Expr& exprIn = (*store)[in];

        if (exprIn.op == OpType::Xor && exprIn.inputs.size() >= 2) {
            xorNodeID = in;
            foundXor = true;
            break;
        }
    }

    if (!foundXor) {
        BF_CORE_ASSERT(false);
        return id;
    }

    const auto xorInputs = (*store)[xorNodeID].inputs;

    ExprInputs others;
    others.reserve(inputs.size());

    for (auto in : inputs) {
        if (in != xorNodeID)
            others.push_back(in);
    }

    ExprInputs distributed;
    distributed.reserve(xorInputs.size());

    for (auto term : xorInputs) {
        ExprId acc = term;

        for (auto other : others)
            acc = BuildAnd(store, acc, other);

        distributed.push_back(acc);
    }

    if (distributed.empty())
        return ctx.replace(id, store->makeFalse(bitWidth).id);

    if (distributed.size() == 1)
        return ctx.replace(id, distributed[0]);

    return ctx.replace(id, store->create(OpType::Xor, std::move(distributed), bitWidth).id);
}

Rule Get_Distribute_Rule() {
    return Rule{Distribute, &Match_Distribute_And_Over_Xor, &Rewrite_Distribute_And_Over_Xor, {Normalize::Flatten}};
}
} // namespace BitFlow::Engine::Core::Rules::Factorize::Bitwise
