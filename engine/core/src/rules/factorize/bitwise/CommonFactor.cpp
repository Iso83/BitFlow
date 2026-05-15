#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static ExprId FindBestCommonFactor(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        std::vector<ExprId> seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);
            counts[inId]++;
        }
    }

    ExprId best;
    size_t bestCount = 0;
    bool hasBest = false;

    for (const auto& [factorId, count] : counts) {
        if (count < 2)
            continue;

        if (!hasBest || count > bestCount) {
            best = factorId;
            bestCount = count;
            hasBest = true;
        }
    }

    if (!hasBest) {
        _ASSERT(false);
        return id;
    }

    return best;
}

static bool Match_XorAnd(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        std::vector<ExprId> seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);

            auto& cnt = counts[inId];
            if (++cnt >= 2)
                return true;
        }
    }

    return false;
}

static ExprId Rewrite_XorAnd(ExprStore* store, ExprId id) {
    const Expr expr = (*store)[id];

    const ExprId bestFactor = FindBestCommonFactor(store, id);

    std::vector<ExprId> termsToFactor;
    termsToFactor.reserve(expr.inputs.size());

    for (auto termId : expr.inputs) {
        const Expr term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        for (auto inId : term.inputs) {
            if (inId == bestFactor) {
                termsToFactor.push_back(termId);
                break;
            }
        }
    }

    if (termsToFactor.size() < 2) {
        _ASSERT(false);
        return id;
    }

    std::vector<ExprId> xorInputs;
    xorInputs.reserve(termsToFactor.size());

    for (auto termId : termsToFactor) {
        const Expr term = (*store)[termId];

        std::vector<ExprId> rest;
        rest.reserve(term.inputs.size());

        bool removed = false;

        for (auto inId : term.inputs) {
            if (!removed && inId == bestFactor) {
                removed = true;
                continue;
            }

            rest.push_back(inId);
        }

        if (rest.empty()) {
            xorInputs.push_back(store->makeTrue(term.bitWidth).id);
        } else if (rest.size() == 1) {
            xorInputs.push_back(rest[0]);
        } else {
            xorInputs.push_back(store->create(OpType::And, std::move(rest), term.bitWidth).id);
        }
    }

    ExprId xorExpr{};

    if (xorInputs.size() == 1) {
        xorExpr = xorInputs[0];
    } else {
        xorExpr = store->create(OpType::Xor, std::move(xorInputs), expr.bitWidth).id;
    }

    ExprId factored = store->create(OpType::And, {bestFactor, xorExpr}, expr.bitWidth).id;

    std::vector<ExprId> finalInputs;
    finalInputs.reserve(expr.inputs.size() - termsToFactor.size() + 1);

    finalInputs.push_back(factored);

    for (auto termId : expr.inputs) {
        if (std::find(termsToFactor.begin(), termsToFactor.end(), termId) == termsToFactor.end()) {
            finalInputs.push_back(termId);
        }
    }

    if (finalInputs.size() == 1)
        return finalInputs[0];

    return store->create(OpType::Xor, std::move(finalInputs), expr.bitWidth).id;
}

Rule Get_XorAnd_Rule() {
    return Rule{XorAnd, &Match_XorAnd, &Rewrite_XorAnd, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise