#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_AndCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        counts[in]++;

        if (counts[in] >= 2)
            return true;
    }

    return false;
}

static bool Match_OrCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        counts[in]++;

        if (counts[in] >= 2)
            return true;
    }

    return false;
}

static bool Match_XorCancel(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<Ids::ExprId, size_t> counts{};

    bool hasZero = false;
    bool hasCancelablePair = false;

    uint64_t constParity = 0;
    size_t constCount = 0;

    for (auto in : e.inputs) {
        const Expr& child = (*store)[in];

        if (child.op == OpType::Const && store->isFalse(in)) {
            hasZero = true;
            continue;
        }

        if (child.op == OpType::Const) {
            constParity ^= child.knownValue;
            constCount++;
            continue;
        }

        auto& count = counts[in];
        count++;

        if (count >= 2)
            hasCancelablePair = true;
    }

    if (hasZero)
        return true;

    if (hasCancelablePair)
        return true;

    // 1 ^ 1 -> 0
    if (constCount >= 2 && constParity == 0)
        return true;

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AndCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<ExprId, bool> seen;
    seen.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        if (seen[in])
            continue;

        seen[in] = true;
        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeTrue(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_OrCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<ExprId, bool> seen;
    seen.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        if (seen[in])
            continue;

        seen[in] = true;
        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_XorCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    Types::ExprChunk constParity = 0;
    bool hasConst = false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            constParity ^= in.knownValue;
            hasConst = true;
        } else {
            counts[inId]++;
        }
    }

    constParity &= mask;

    std::vector<ExprId> terms;
    terms.reserve(counts.size() + 1);

    for (const auto& [exprId, count] : counts) {
        if (count & 1)
            terms.push_back(exprId);
    }

    if (hasConst && constParity != 0)
        terms.push_back(store->createConstant(constParity, e.bitWidth).id);

    std::sort(terms.begin(), terms.end(), [&](ExprId a, ExprId b) { return CompareExprCanonical(store, a, b) < 0; });

    if (terms.empty())
        return store->zeroId();
    if (terms.size() == 1)
        return terms[0];

    return store->create(OpType::Xor, std::move(terms), e.bitWidth).id;
}
#pragma endregion

Rule Get_AndCancel_Rule() {
    return Rule{AndCancel, &Match_AndCancel, &Rewrite_AndCancel, {Normalize::Flatten}};
}

Rule Get_OrCancel_Rule() {
    return Rule{OrCancel, &Match_OrCancel, &Rewrite_OrCancel, {Normalize::Order}};
}

Rule Get_XorCancel_Rule() {
    return Rule{XorCancel, &Match_XorCancel, &Rewrite_XorCancel, {Normalize::Order, XorZero}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
