#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_And_Cancel(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

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

static bool Match_Or_Cancel(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

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

static bool Match_XorCancel(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    const uint64_t mask = Expr::fullMask(e.bitWidth);

    uint64_t constParity = 0;
    bool hasConst = false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (ExprId inId : e.inputs) {
        const Expr& in = store->get(inId);

        if (in.op == OpType::Const) {
            constParity ^= in.knownValue;
            hasConst = true;
        } else {
            counts[inId]++;

            // x ^ x → 0
            if (counts[inId] >= 2)
                return true;
        }
    }

    constParity &= mask;

    if (hasConst && constParity != 0)
        return true;

    if (hasConst && constParity == 0) {
        if (counts.empty())
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_And_Cancel(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

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

static ExprId Rewrite_Or_Cancel(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

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
    const Expr& e = store->get(id);

    const uint64_t mask = Expr::fullMask(e.bitWidth);

    uint64_t constParity = 0;
    bool hasConst = false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (ExprId inId : e.inputs) {
        const Expr& in = store->get(inId);

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

    return MakeXor(store, terms, e.bitWidth);
}
#pragma endregion

Rule Get_And_Cancel_Rule() {
    return Rule{And_Cancel, &Match_And_Cancel, &Rewrite_And_Cancel, {Normalize::Flatten}};
}

Rule Get_Or_Cancel_Rule() {
    return Rule{Or_Cancel, &Match_Or_Cancel, &Rewrite_Or_Cancel, {Normalize::Flatten}};
}

Rule Get_Xor_Cancel_Rule() {
    return Rule{Xor_Cancel, &Match_XorCancel, &Rewrite_XorCancel, {Normalize::Flatten, Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
