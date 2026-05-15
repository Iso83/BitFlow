#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static void AddCoeff(std::unordered_map<ExprId, Types::ExprChunk>& coeffByBaseId, std::vector<ExprId>& baseOrder,
                     ExprId base, Types::ExprChunk coeff, Types::ExprChunk mask) {
    if (coeffByBaseId.emplace(base, 0).second)
        baseOrder.push_back(base);

    coeffByBaseId[base] = (coeffByBaseId[base] + coeff) & mask;
}

struct LinearTerm {
    ExprId base{};
    Types::ExprChunk coeff{0};
};

static bool DecomposeLinearTerm(const ExprStore* store, ExprId termId, LinearTerm& out, Types::BitWidth bitWidth) {
    const Expr& term = (*store)[termId];

    if (term.op == OpType::Const)
        return false;

    if (term.op != OpType::Mul) {
        out.base = termId;
        out.coeff = 1;
        return true;
    }

    Types::ExprChunk coeff = 1;

    ExprId base;
    bool hasBase = false;
    bool sawConst = false;

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    for (ExprId factorId : term.inputs) {
        const Expr& factor = (*store)[factorId];

        if (factor.op == OpType::Const) {
            coeff = (coeff * factor.knownValue) & mask;
            sawConst = true;
            continue;
        }

        if (hasBase)
            return false;

        base = factorId;
        hasBase = true;
    }

    if (!sawConst || !hasBase)
        return false;

    out.base = base;
    out.coeff = coeff;
    return true;
}

#pragma region Match
static bool Match_AddLinearMultiplicity(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> baseTermCounts;
    baseTermCounts.reserve(e.inputs.size());

    for (ExprId inputId : e.inputs) {

        const Expr& term = (*store)[inputId];

        // implicit coeff = 1
        if (term.op != OpType::Mul && term.op != OpType::Const) {

            if (++baseTermCounts[inputId] >= 2)
                return true;

            continue;
        }

        LinearTerm linear{};
        if (!DecomposeLinearTerm(store, inputId, linear, e.bitWidth))
            continue;

        if (++baseTermCounts[linear.base] >= 2)
            return true;
    }

    return false;
}

static bool Match_AddCommonFactor(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& lhs = (*store)[e.inputs[i]];
        if (lhs.op != OpType::Mul || lhs.inputs.size() != 2)
            continue;

        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr& rhs = (*store)[e.inputs[j]];
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
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddLinearMultiplicity(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const auto originalInputs = e.inputs;
    const auto bitWidth = e.bitWidth;

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    std::unordered_map<ExprId, Types::ExprChunk> coeffByBaseId;

    std::vector<ExprId> baseOrder;
    baseOrder.reserve(originalInputs.size());

    std::vector<ExprId> passthroughTerms;
    passthroughTerms.reserve(originalInputs.size());

    for (ExprId inputId : originalInputs) {
        const Expr& term = (*store)[inputId];

        // implicit coeff = 1
        if (term.op != OpType::Mul && term.op != OpType::Const) {
            AddCoeff(coeffByBaseId, baseOrder, inputId, 1, mask);
            continue;
        }

        LinearTerm linear{};

        if (!DecomposeLinearTerm(store, inputId, linear, bitWidth)) {
            passthroughTerms.push_back(inputId);
            continue;
        }

        AddCoeff(coeffByBaseId, baseOrder, linear.base, linear.coeff, mask);
    }

    std::vector<ExprId> normalizedAddTerms;
    normalizedAddTerms.reserve(baseOrder.size() + passthroughTerms.size());

    for (ExprId baseId : baseOrder) {
        const Types::ExprChunk coeff = coeffByBaseId[baseId] & mask;

        if (coeff == 0)
            continue;

        if (coeff == 1) {
            normalizedAddTerms.push_back(baseId);
            continue;
        }

        const ExprId coeffId = store->createConstant(coeff, bitWidth).id;

        normalizedAddTerms.push_back(store->create(OpType::Mul, {baseId, coeffId}, bitWidth).id);
    }

    for (ExprId termId : passthroughTerms)
        normalizedAddTerms.push_back(termId);

    if (normalizedAddTerms.empty())
        return store->createConstant(0, bitWidth).id;

    // no-op detection
    if (normalizedAddTerms.size() == originalInputs.size()) {

        bool identical = true;

        for (size_t i = 0; i < originalInputs.size(); ++i) {
            if (normalizedAddTerms[i] != originalInputs[i]) {
                identical = false;
                break;
            }
        }

        if (identical)
            return id;
    }

    if (normalizedAddTerms.size() == 1)
        return normalizedAddTerms[0];

    return store->create(OpType::Add, std::move(normalizedAddTerms), bitWidth).id;
}

static ExprId Rewrite_AddCommonFactor(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::unordered_map<ExprId, int> factorFrequency;
    factorFrequency.reserve(e.inputs.size() * 2);

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::Mul || term.inputs.size() != 2)
            continue;

        factorFrequency[term.inputs[0]]++;
        factorFrequency[term.inputs[1]]++;
    }

    ExprId common;
    bool hasCommon = false;
    int bestFrequency = 0;

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::Mul || term.inputs.size() != 2)
            continue;

        for (ExprId factorId : term.inputs) {
            const int frequency = factorFrequency[factorId];

            if (!hasCommon || frequency > bestFrequency) {
                bestFrequency = frequency;
                common = factorId;
                hasCommon = true;
            }
        }
    }

    if (!hasCommon || bestFrequency < 2)
        return id;

    std::vector<ExprId> sharedInnerTerms;
    std::vector<ExprId> untouchedTerms;

    sharedInnerTerms.reserve(e.inputs.size());
    untouchedTerms.reserve(e.inputs.size());

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

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
#pragma endregion

Rule Get_AddLinearMultiplicity_Rule() {
    return Rule{
        AddLinearMultiplicity, &Match_AddLinearMultiplicity, &Rewrite_AddLinearMultiplicity, {Normalize::Order}};
}

Rule Get_AddCommonFactor_Rule() {
    return Rule{AddCommonFactor, &Match_AddCommonFactor, &Rewrite_AddCommonFactor, {AddLinearMultiplicity}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
