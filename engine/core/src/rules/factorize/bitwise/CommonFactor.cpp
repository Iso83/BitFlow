#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RewriteCost.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using Expr = Expression::Expr;
using OpType = Expression::OpType;
using namespace BitFlow::Core::Expression;

static void CollectFactors(Expr* e, std::vector<Expr*>& out) {
    if (e->op == OpType::And) {
        for (auto* in : e->inputs)
            out.push_back(in);
    } else
        out.push_back(e);
}

static Expr* BuildAndNode(const std::vector<Expr*>& terms) {
    if (terms.empty())
        return ConstPool::Get(1);

    if (terms.size() == 1)
        return terms[0];

    return MakeOpInterned(OpType::And, terms);
}

static Expr* BuildXorNode(const std::vector<Expr*>& terms) {
    if (terms.empty())
        return ConstPool::Get(0);

    if (terms.size() == 1)
        return terms[0];

    return MakeOpInterned(OpType::Xor, terms);
}

static uint32_t FindBestCommonFactorId(const Expr& e) {
    std::unordered_map<uint32_t, size_t> counts;

    for (const Expr* term : e.inputs) {
        if (term->op != OpType::And || term->inputs.size() < 2)
            continue;

        std::vector<uint32_t> seenInBranch;
        seenInBranch.reserve(term->inputs.size());

        for (const Expr* in : term->inputs) {
            const uint32_t id = in->id.value();

            if (std::find(seenInBranch.begin(), seenInBranch.end(), id) != seenInBranch.end())
                continue;

            seenInBranch.push_back(id);
            counts[id]++;
        }
    }

    uint32_t bestId = 0;
    size_t bestCount = 0;

    for (const auto& [id, count] : counts) {
        if (count < 2)
            continue;

        if (bestId == 0 || count > bestCount || (count == bestCount && id < bestId)) {
            bestId = id;
            bestCount = count;
        }
    }

    return bestId;
}

static bool Match_Xor_And(const Expr& e) {
    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    std::unordered_map<Expr*, int> factorCount;

    for (Expr* term : e.inputs) {
        std::vector<Expr*> factors;
        CollectFactors(term, factors);

        for (Expr* f : factors) {
            factorCount[f]++;
            if (factorCount[f] >= 2)
                return true;
        }
    }

    return false;
}

static Expr* Rewrite_Xor_And(Expr& e) {
    const uint32_t bestFactorId = FindBestCommonFactorId(e);
    if (bestFactorId == 0)
        return nullptr;

    Expr* common = nullptr;
    std::vector<Expr*> termsToFactor;
    termsToFactor.reserve(e.inputs.size());

    for (Expr* term : e.inputs) {
        if (term->op != OpType::And || term->inputs.size() < 2)
            continue;

        bool hasBestFactor = false;
        for (Expr* in : term->inputs) {
            if (in->id.value() == bestFactorId) {
                hasBestFactor = true;
                common = in;
                break;
            }
        }

        if (hasBestFactor)
            termsToFactor.push_back(term);
    }

    if (common == nullptr || termsToFactor.size() < 2)
        return nullptr;

    std::vector<Expr*> newXorInputs;
    newXorInputs.reserve(termsToFactor.size());

    for (Expr* term : termsToFactor) {
        std::vector<Expr*> rest;
        rest.reserve(term->inputs.size());

        bool removed = false;
        for (Expr* in : term->inputs) {
            if (!removed && in->id.value() == bestFactorId) {
                removed = true;
                continue;
            }

            rest.push_back(in);
        }

        if (rest.empty())
            newXorInputs.push_back(ConstPool::Get(1));
        else
            newXorInputs.push_back(BuildAndNode(rest));
    }

    Expr* newXor = BuildXorNode(newXorInputs);
    Expr* newAnd = Expression::MakeOpInterned(OpType::And, {common, newXor});

    std::vector<Expr*> finalInputs;
    finalInputs.reserve(e.inputs.size());
    finalInputs.push_back(newAnd);

    for (Expr* term : e.inputs) {
        bool isFactored = false;
        for (Expr* candidate : termsToFactor) {
            if (candidate == term) {
                isFactored = true;
                break;
            }
        }

        if (!isFactored)
            finalInputs.push_back(term);
    }

    Expr* candidate = BuildXorNode(finalInputs);
    if (!IsRewritePreferred(candidate, &e, RewriteCostPolicy::FactorizeSafe))
        return nullptr;

    return candidate;
}

Rule Get_Xor_And_Rule() {
    return Rule{RuleId::Factorize_XorAnd,
                &Match_Xor_And,
                &Rewrite_Xor_And,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::Factorizing,
                "Factorize_XorAnd"};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise
