#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RewriteCost.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Expression;

struct LinearTerm {
    const ExprOld* base = nullptr;
    uint32_t coeff = 0;
};

static bool DecomposeLinearTerm(const ExprOld* term, LinearTerm& out) {
    if (term->op == OpType::Const)
        return false;

    if (term->op != OpType::Mul) {
        out.base = term;
        out.coeff = 1u;
        return true;
    }

    uint32_t coeff = 1u;
    const ExprOld* base = nullptr;
    bool sawConst = false;
    for (ExprOld* factor : term->inputs) {
        if (factor->op == OpType::Const) {
            coeff *= factor->constValue;
            sawConst = true;
            continue;
        }

        if (base != nullptr)
            return false;
        base = factor;
    }

    if (!sawConst || base == nullptr)
        return false;

    out.base = base;
    out.coeff = coeff;
    return true;
}

static bool Match_Add_LinearMultiplicity(const ExprOld& e) {
    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> baseTermCounts;
    for (const ExprOld* input : e.inputs) {
        LinearTerm linear{};
        if (!DecomposeLinearTerm(input, linear))
            continue;

        if (++baseTermCounts[linear.base->id.value()] >= 2)
            return true;
    }

    return false;
}

static ExprOld* Rewrite_Add_LinearMultiplicity(ExprOld& e) {
    std::unordered_map<uint32_t, uint32_t> coeffByBaseId;
    std::unordered_map<uint32_t, ExprOld*> baseExprById;
    std::vector<uint32_t> baseOrder;
    baseOrder.reserve(e.inputs.size());
    std::vector<ExprOld*> passthroughTerms;
    passthroughTerms.reserve(e.inputs.size());

    for (ExprOld* input : e.inputs) {
        LinearTerm linear{};
        if (!DecomposeLinearTerm(input, linear)) {
            passthroughTerms.push_back(input);
            continue;
        }

        const uint32_t baseId = linear.base->id.value();
        if (coeffByBaseId.emplace(baseId, 0u).second) {
            baseOrder.push_back(baseId);
            baseExprById[baseId] = const_cast<ExprOld*>(linear.base);
        }
        coeffByBaseId[baseId] += linear.coeff;
    }

    std::vector<ExprOld*> normalizedAddTerms;
    normalizedAddTerms.reserve(baseOrder.size() + passthroughTerms.size());
    for (uint32_t baseId : baseOrder) {
        ExprOld* base = baseExprById[baseId];
        const uint32_t coeff = coeffByBaseId[baseId];
        if (coeff == 0u)
            continue;
        if (coeff == 1u) {
            normalizedAddTerms.push_back(base);
            continue;
        }
        normalizedAddTerms.push_back(MakeOpInterned(OpType::Mul, {base, ConstPool::Get(coeff)}));
    }
    for (ExprOld* term : passthroughTerms)
        normalizedAddTerms.push_back(term);

    if (normalizedAddTerms.empty())
        return nullptr;

    const bool mergedLinearTerms = normalizedAddTerms.size() < e.inputs.size();
    if (!mergedLinearTerms)
        return nullptr;

    ExprOld* candidate =
        (normalizedAddTerms.size() == 1) ? normalizedAddTerms[0] : MakeOpInterned(OpType::Add, normalizedAddTerms);
    return candidate;
}

static bool Match_Add_CommonFactor(const ExprOld& e) {
    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const ExprOld* lhs = e.inputs[i];
        if (lhs->op != OpType::Mul || lhs->inputs.size() != 2)
            continue;

        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            const ExprOld* rhs = e.inputs[j];
            if (rhs->op != OpType::Mul || rhs->inputs.size() != 2)
                continue;

            for (const ExprOld* l : lhs->inputs) {
                for (const ExprOld* r : rhs->inputs) {
                    if (l->id == r->id)
                        return true;
                }
            }
        }
    }

    return false;
}

static ExprOld* Rewrite_Add_CommonFactor(ExprOld& e) {
    std::vector<ExprOld*> addTerms = e.inputs;

    std::unordered_map<ExprOld*, int> factorFrequency;
    for (ExprOld* term : addTerms) {
        if (term->op != OpType::Mul || term->inputs.size() != 2)
            continue;
        factorFrequency[term->inputs[0]]++;
        factorFrequency[term->inputs[1]]++;
    }

    ExprOld* common = nullptr;
    int bestFrequency = 0;
    for (ExprOld* term : addTerms) {
        if (term->op != OpType::Mul || term->inputs.size() != 2)
            continue;
        for (ExprOld* factor : term->inputs) {
            const int frequency = factorFrequency[factor];
            if (frequency > bestFrequency) {
                bestFrequency = frequency;
                common = factor;
            }
        }
    }

    if (!common || bestFrequency < 2)
        return nullptr;

    std::vector<ExprOld*> sharedInnerTerms;
    std::vector<ExprOld*> untouchedTerms;
    sharedInnerTerms.reserve(addTerms.size());
    untouchedTerms.reserve(addTerms.size());

    for (ExprOld* term : addTerms) {
        if (term->op == OpType::Mul && term->inputs.size() == 2) {
            if (term->inputs[0]->id == common->id) {
                sharedInnerTerms.push_back(term->inputs[1]);
                continue;
            }
            if (term->inputs[1]->id == common->id) {
                sharedInnerTerms.push_back(term->inputs[0]);
                continue;
            }
        }
        untouchedTerms.push_back(term);
    }

    if (sharedInnerTerms.size() < 2)
        return nullptr;

    ExprOld* innerAdd = MakeOpInterned(OpType::Add, sharedInnerTerms);
    ExprOld* factored = MakeOpInterned(OpType::Mul, {common, innerAdd});

    std::vector<ExprOld*> finalAddTerms;
    finalAddTerms.reserve(untouchedTerms.size() + 1);
    for (ExprOld* term : untouchedTerms)
        finalAddTerms.push_back(term);
    finalAddTerms.push_back(factored);

    ExprOld* candidate = (finalAddTerms.size() == 1) ? finalAddTerms[0] : MakeOpInterned(OpType::Add, finalAddTerms);
    if (!IsRewritePreferred(candidate, &e, RewriteCostPolicy::FactorizeSafe))
        return nullptr;

    return candidate;
}

Rule Get_Add_LinearMultiplicity_Rule() {
    return Rule{RuleId::Factorize_AddLinearMultiplicity,
                &Match_Add_LinearMultiplicity,
                &Rewrite_Add_LinearMultiplicity,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::Factorizing | RuleFlags::Arithmetic,
                "Factorize_AddLinearMultiplicity"};
}

Rule Get_Add_CommonFactor_Rule() {
    return Rule{RuleId::Factorize_AddCommonFactor,
                &Match_Add_CommonFactor,
                &Rewrite_Add_CommonFactor,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order, RuleId::Factorize_AddLinearMultiplicity},
                RuleFlags::Factorizing | RuleFlags::Arithmetic,
                "Factorize_AddCommonFactor"};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
