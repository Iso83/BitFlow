#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

struct LinearTerm {
    ExprId base{};
    uint64_t coeff{0};
};

static bool DecomposeLinearTerm(const ExprStore* store, ExprId termId, LinearTerm& out, uint16_t bitWidth) {
    const Expr& term = store->get(termId);

    if (term.op == OpType::Const)
        return false;

    if (term.op != OpType::Mul) {
        out.base = termId;
        out.coeff = 1;
        return true;
    }

    uint64_t coeff = 1;
    ExprId base{};
    bool sawConst = false;

    const uint64_t mask = Expr::fullMask(bitWidth);

    for (ExprId factorId : term.inputs) {
        const Expr& factor = store->get(factorId);

        if (factor.op == OpType::Const) {
            coeff = (coeff * factor.knownValue) & mask;
            sawConst = true;
            continue;
        }

        if (base != ExprId{})
            return false;

        base = factorId;
    }

    if (!sawConst || base == ExprId{})
        return false;

    out.base = base;
    out.coeff = coeff;
    return true;
}

static bool Match_Add_LinearMultiplicity(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> baseTermCounts;
    baseTermCounts.reserve(e.inputs.size());

    for (ExprId inputId : e.inputs) {
        LinearTerm linear{};
        if (!DecomposeLinearTerm(store, inputId, linear, e.bitWidth))
            continue;

        if (++baseTermCounts[linear.base] >= 2)
            return true;
    }

    return false;
}

static ExprId Rewrite_Add_LinearMultiplicity(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    const uint64_t mask = Expr::fullMask(e.bitWidth);

    std::unordered_map<ExprId, uint64_t> coeffByBaseId;
    std::vector<ExprId> baseOrder;
    baseOrder.reserve(e.inputs.size());

    std::vector<ExprId> passthroughTerms;
    passthroughTerms.reserve(e.inputs.size());

    for (ExprId inputId : e.inputs) {
        LinearTerm linear{};
        if (!DecomposeLinearTerm(store, inputId, linear, e.bitWidth)) {
            passthroughTerms.push_back(inputId);
            continue;
        }

        if (coeffByBaseId.emplace(linear.base, 0).second)
            baseOrder.push_back(linear.base);

        coeffByBaseId[linear.base] = (coeffByBaseId[linear.base] + linear.coeff) & mask;
    }

    std::vector<ExprId> normalizedAddTerms;
    normalizedAddTerms.reserve(baseOrder.size() + passthroughTerms.size());

    for (ExprId baseId : baseOrder) {
        const uint64_t coeff = coeffByBaseId[baseId] & mask;

        if (coeff == 0)
            continue;

        if (coeff == 1) {
            normalizedAddTerms.push_back(baseId);
            continue;
        }

        const ExprId coeffId = store->createConstant(coeff, e.bitWidth).id;
        normalizedAddTerms.push_back(store->create(OpType::Mul, {baseId, coeffId}, e.bitWidth).id);
    }

    for (ExprId termId : passthroughTerms)
        normalizedAddTerms.push_back(termId);

    if (normalizedAddTerms.empty())
        return store->createConstant(0, e.bitWidth).id;

    const bool mergedLinearTerms = normalizedAddTerms.size() < e.inputs.size();
    if (!mergedLinearTerms)
        return id;

    if (normalizedAddTerms.size() == 1)
        return normalizedAddTerms[0];

    return store->create(OpType::Add, std::move(normalizedAddTerms), e.bitWidth).id;
}

static bool Match_Add_CommonFactor(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& lhs = store->get(e.inputs[i]);
        if (lhs.op != OpType::Mul || lhs.inputs.size() != 2)
            continue;

        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr& rhs = store->get(e.inputs[j]);
            if (rhs.op != OpType::Mul || rhs.inputs.size() != 2)
                continue;

            for (ExprId l : lhs.inputs) {
                for (ExprId r : rhs.inputs) {
                    if (l == r)
                        return true;
                }
            }
        }
    }

    return false;
}

static ExprId Rewrite_Add_CommonFactor(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    std::unordered_map<ExprId, int> factorFrequency;
    factorFrequency.reserve(e.inputs.size() * 2);

    for (ExprId termId : e.inputs) {
        const Expr& term = store->get(termId);

        if (term.op != OpType::Mul || term.inputs.size() != 2)
            continue;

        factorFrequency[term.inputs[0]]++;
        factorFrequency[term.inputs[1]]++;
    }

    ExprId common{};
    int bestFrequency = 0;

    for (ExprId termId : e.inputs) {
        const Expr& term = store->get(termId);

        if (term.op != OpType::Mul || term.inputs.size() != 2)
            continue;

        for (ExprId factorId : term.inputs) {
            const int frequency = factorFrequency[factorId];
            if (frequency > bestFrequency) {
                bestFrequency = frequency;
                common = factorId;
            }
        }
    }

    if (common == ExprId{} || bestFrequency < 2)
        return id;

    std::vector<ExprId> sharedInnerTerms;
    std::vector<ExprId> untouchedTerms;

    sharedInnerTerms.reserve(e.inputs.size());
    untouchedTerms.reserve(e.inputs.size());

    for (ExprId termId : e.inputs) {
        const Expr& term = store->get(termId);

        if (term.op == OpType::Mul && term.inputs.size() == 2) {
            if (term.inputs[0] == common) {
                sharedInnerTerms.push_back(term.inputs[1]);
                continue;
            }

            if (term.inputs[1] == common) {
                sharedInnerTerms.push_back(term.inputs[0]);
                continue;
            }
        }

        untouchedTerms.push_back(termId);
    }

    if (sharedInnerTerms.size() < 2)
        return id;

    const ExprId innerAdd = sharedInnerTerms.size() == 1
                                ? sharedInnerTerms[0]
                                : store->create(OpType::Add, std::move(sharedInnerTerms), e.bitWidth).id;

    const ExprId factored = store->create(OpType::Mul, {common, innerAdd}, e.bitWidth).id;

    std::vector<ExprId> finalAddTerms;
    finalAddTerms.reserve(untouchedTerms.size() + 1);

    for (ExprId termId : untouchedTerms)
        finalAddTerms.push_back(termId);

    finalAddTerms.push_back(factored);

    if (finalAddTerms.size() == 1)
        return finalAddTerms[0];

    return store->create(OpType::Add, std::move(finalAddTerms), e.bitWidth).id;
}

Rule Get_Add_Linear_Multiplicity_Rule() {
    return Rule{Add_Linear_Multiplicity,
                &Match_Add_LinearMultiplicity,
                &Rewrite_Add_LinearMultiplicity,
                {Normalize::Flatten, Normalize::Order}};
}

Rule Get_Add_CommonFactor_Rule() {
    return Rule{Add_CommonFactor,
                &Match_Add_CommonFactor,
                &Rewrite_Add_CommonFactor,
                {Normalize::Flatten, Normalize::Order, Add_Linear_Multiplicity}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
