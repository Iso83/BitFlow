#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool ContainsStructurallyEquivalent(const ExprStore* store, const std::vector<ExprId>& ids, ExprId target) {
    for (ExprId id : ids) {
        if (store->structuralEquivalent(id, target))
            return true;
    }

    return false;
}

static bool Match_XorPairCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> childCounts;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Xor || exprIn.inputs.size() < 2)
            continue;

        std::vector<ExprId> seenInChild;
        seenInChild.reserve(exprIn.inputs.size());

        for (auto term : exprIn.inputs) {
            if (ContainsStructurallyEquivalent(store, seenInChild, term))
                continue;

            seenInChild.push_back(term);
            childCounts[term]++;

            if (childCounts[term] >= 2)
                return true;
        }
    }

    return false;
}

static ExprId Rewrite_XorPairCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::unordered_map<ExprId, int> childCounts;
    std::unordered_map<ExprId, ExprId> firstSeen;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Xor || exprIn.inputs.size() < 2)
            continue;

        std::vector<ExprId> seenInChild;
        seenInChild.reserve(exprIn.inputs.size());

        for (auto term : exprIn.inputs) {
            if (ContainsStructurallyEquivalent(store, seenInChild, term))
                continue;

            seenInChild.push_back(term);
            childCounts[term]++;

            if (!firstSeen.count(term))
                firstSeen[term] = term;
        }
    }

    ExprId commonId;
    ExprId common;
    bool hasCommon = false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op != OpType::Xor || exprIn.inputs.size() < 2)
            continue;

        for (auto term : exprIn.inputs) {
            if (childCounts[term] >= 2) {
                commonId = term;
                common = firstSeen[term];
                hasCommon = true;
                break;
            }
        }

        if (hasCommon)
            break;
    }

    if (!hasCommon) {
        BF_CORE_ASSERT(false);
        return id;
    }

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    int matchedChildren = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Xor && exprIn.inputs.size() >= 2 && ContainsExpr(store, in, commonId)) {
            matchedChildren++;

            std::vector<ExprId> residual;
            residual.reserve(exprIn.inputs.size());

            for (auto term : exprIn.inputs) {
                if (!store->structuralEquivalent(term, commonId))
                    residual.push_back(term);
            }

            if (residual.empty())
                newInputs.push_back(store->makeFalse(e.bitWidth).id);
        } else
            newInputs.push_back(in);
    }

    if ((matchedChildren & 1) != 0)
        newInputs.push_back(common);

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Xor, std::move(newInputs), e.bitWidth).id;
}

Rule Get_XorPairCancel_Rule() {
    return Rule{XorPairCancel, &Match_XorPairCancel, &Rewrite_XorPairCancel, {Simplify::Bitwise::XorCancel}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise
