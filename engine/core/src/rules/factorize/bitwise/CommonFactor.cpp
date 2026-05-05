#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_And(const ExprStore* eStore, ExprId id) {
    const Expr& e = eStore->get(id);

    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> factorCount;

    for (auto termId : e.inputs) {
        const auto& term = eStore->get(termId);

        if (term.op == OpType::And) {
            for (auto f : term.inputs) {
                auto& cnt = factorCount[f];
                if (++cnt >= 2)
                    return true;
            }
        } else {
            auto& cnt = factorCount[termId];
            if (++cnt >= 2)
                return true;
        }
    }

    return false;
}

static ExprId FindBestCommonFactor(const ExprStore* eStore, ExprId id) {
    const Expr& e = eStore->get(id);

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const auto& term = eStore->get(termId);

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

    ExprId best = id;
    size_t bestCount = 0;
    bool hasBest = false;

    for (const auto& [id, count] : counts) {
        if (count < 2)
            continue;

        if (!hasBest || count > bestCount) {
            best = id;
            bestCount = count;
            hasBest = true;
        }
    }

    return hasBest ? best : id;
}

static ExprId Rewrite_Xor_And(ExprStore* eStore, ExprId id) {
    const Expr& e = eStore->get(id);

    const ExprId bestFactor = FindBestCommonFactor(eStore, id);
    if (bestFactor == id) {
        _ASSERT(false);
        return id;
    }

    ExprId common{};
    std::vector<ExprId> termsToFactor;
    termsToFactor.reserve(e.inputs.size());

    for (auto termId : e.inputs) {
        const auto& term = eStore->get(termId);

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        for (auto inId : term.inputs) {
            if (inId == bestFactor) {
                common = inId;
                termsToFactor.push_back(termId);
                break;
            }
        }
    }

    if (termsToFactor.size() < 2) {
        _ASSERT(false);
        return id;
    }

    std::vector<ExprId> newXorInputs;
    newXorInputs.reserve(termsToFactor.size());

    for (auto termId : termsToFactor) {
        const auto& term = eStore->get(termId);

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

        if (rest.empty())
            newXorInputs.push_back(eStore->makeTrue(term.bitWidth).id);
        else if (rest.size() == 1)
            newXorInputs.push_back(rest[0]);
        else
            newXorInputs.push_back(eStore->create(OpType::And, std::move(rest), term.bitWidth).id);
    }

    ExprId newXor;
    if (newXorInputs.size() == 1)
        newXor = newXorInputs[0];
    else
        newXor = eStore->create(OpType::Xor, std::move(newXorInputs), e.bitWidth).id;

    ExprId newAnd = eStore->create(OpType::And, {common, newXor}, e.bitWidth).id;

    std::vector<ExprId> finalInputs;
    finalInputs.reserve(e.inputs.size());
    finalInputs.push_back(newAnd);

    for (auto termId : e.inputs) {
        if (std::find(termsToFactor.begin(), termsToFactor.end(), termId) == termsToFactor.end())
            finalInputs.push_back(termId);
    }

    if (finalInputs.size() == 1)
        return finalInputs[0];

    return eStore->create(OpType::Xor, std::move(finalInputs), e.bitWidth).id;
}

Rule Get_Xor_And_Rule() {
    return Rule{Xor_And, &Match_Xor_And, &Rewrite_Xor_And, {Normalize::Flatten, Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise