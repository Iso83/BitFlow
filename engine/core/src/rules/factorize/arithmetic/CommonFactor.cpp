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

    if (term.op == OpType::Sub)
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
        if (term.op != OpType::Mul && term.op != OpType::Const && term.op != OpType::Sub) {

            if (++baseTermCounts[inputId] >= 2)
                return true;

            continue;
        }

        LinearTerm linear{};
        if (!DecomposeLinearTerm(store, inputId, linear, e.bitWidth)) {
            const Expr& maybeSub = (*store)[inputId];
            if (maybeSub.op == OpType::Sub && maybeSub.inputs.size() == 2) {
                const Expr& subLhs = (*store)[maybeSub.inputs[0]];
                const Expr& subRhs = (*store)[maybeSub.inputs[1]];
                if (subLhs.op != OpType::Const && subRhs.op == OpType::Const) {
                    if (++baseTermCounts[maybeSub.inputs[0]] >= 2)
                        return true;
                }
            }
            continue;
        }

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
        if (lhs.op != OpType::Mul || lhs.inputs.size() < 2)
            continue;

        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr& rhs = (*store)[e.inputs[j]];
            if (rhs.op != OpType::Mul || rhs.inputs.size() < 2)
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

static bool Match_CommonFactorCancel_PowTerms(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Div || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    if (lhs.op != OpType::Mul || rhs.op != OpType::Mul)
        return false;

    for (ExprId lhsFactorId : lhs.inputs) {
        const Expr& lhsFactor = (*store)[lhsFactorId];
        if (lhsFactor.op != OpType::Pow || lhsFactor.inputs.size() != 2)
            continue;

        const Expr& lhsExp = (*store)[lhsFactor.inputs[1]];
        if (lhsExp.op != OpType::Const)
            continue;

        for (ExprId rhsFactorId : rhs.inputs) {
            const Expr& rhsFactor = (*store)[rhsFactorId];
            if (rhsFactor.op != OpType::Pow || rhsFactor.inputs.size() != 2)
                continue;

            if (lhsFactor.inputs[0] != rhsFactor.inputs[0])
                continue;

            const Expr& rhsExp = (*store)[rhsFactor.inputs[1]];
            if (rhsExp.op != OpType::Const)
                continue;

            if (lhsExp.knownValue == rhsExp.knownValue)
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddLinearMultiplicity(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const std::vector<ExprId> originalInputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    std::unordered_map<ExprId, Types::ExprChunk> coeffByBaseId;

    std::vector<ExprId> baseOrder;
    baseOrder.reserve(originalInputs.size());

    std::vector<ExprId> passthroughTerms;
    passthroughTerms.reserve(originalInputs.size());
    Types::ExprChunk pendingSubConst = 0;

    for (ExprId inputId : originalInputs) {
        const Expr& term = (*store)[inputId];

        // implicit coeff = 1
        if (term.op != OpType::Mul && term.op != OpType::Const && term.op != OpType::Sub) {
            AddCoeff(coeffByBaseId, baseOrder, inputId, 1, mask);
            continue;
        }

        LinearTerm linear{};

        if (!DecomposeLinearTerm(store, inputId, linear, bitWidth)) {
            if (term.op == OpType::Sub && term.inputs.size() == 2) {
                const Expr& subLhs = (*store)[term.inputs[0]];
                const Expr& subRhs = (*store)[term.inputs[1]];
                if (subLhs.op != OpType::Const && subRhs.op == OpType::Const) {
                    AddCoeff(coeffByBaseId, baseOrder, term.inputs[0], 1, mask);
                    pendingSubConst = (pendingSubConst + subRhs.knownValue) & mask;
                    continue;
                }
            }
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

    // no-op detection
    if (pendingSubConst == 0 && normalizedAddTerms.size() == originalInputs.size()) {

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

    ExprId addResult;

    if (normalizedAddTerms.empty())
        addResult = store->zeroId();
    else if (normalizedAddTerms.size() == 1)
        addResult = normalizedAddTerms[0];
    else
        addResult = store->create(OpType::Add, std::move(normalizedAddTerms), bitWidth).id;

    if (pendingSubConst != 0)
        return store->create(OpType::Sub, {addResult, store->createConstant(pendingSubConst, bitWidth).id}, bitWidth)
            .id;

    return addResult;
}

static ExprId Rewrite_AddCommonFactor(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::unordered_map<ExprId, int> factorFrequency;
    factorFrequency.reserve(e.inputs.size() * 2);

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::Mul || term.inputs.size() < 2)
            continue;

        for (ExprId factorId : term.inputs)
            factorFrequency[factorId]++;
    }

    ExprId common;
    bool hasCommon = false;
    int bestFrequency = 0;

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::Mul || term.inputs.size() < 2)
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

        if (term.op == OpType::Mul && term.inputs.size() >= 2) {
            bool hasCommonFactor = false;
            std::vector<ExprId> remainingFactors;
            remainingFactors.reserve(term.inputs.size());

            for (ExprId factorId : term.inputs) {
                if (!hasCommonFactor && factorId == common) {
                    hasCommonFactor = true;
                    continue;
                }
                remainingFactors.push_back(factorId);
            }

            if (hasCommonFactor) {
                if (remainingFactors.empty())
                    sharedInnerTerms.push_back(store->createConstant(1, e.bitWidth).id);
                else if (remainingFactors.size() == 1)
                    sharedInnerTerms.push_back(remainingFactors[0]);
                else
                    sharedInnerTerms.push_back(store->create(OpType::Mul, std::move(remainingFactors), e.bitWidth).id);
                continue;
            }
        }

        untouchedTerms.push_back(termId);
    }

    if (sharedInnerTerms.size() < 2)
        return id;

    const Types::BitWidth bitWidth = e.bitWidth;

    const ExprId innerAdd = sharedInnerTerms.size() == 1
                                ? sharedInnerTerms[0]
                                : store->create(OpType::Add, std::move(sharedInnerTerms), bitWidth).id;

    const ExprId factored = store->create(OpType::Mul, {common, innerAdd}, bitWidth).id;

    std::vector<ExprId> finalAddTerms;
    finalAddTerms.reserve(untouchedTerms.size() + 1);

    for (ExprId termId : untouchedTerms)
        finalAddTerms.push_back(termId);

    finalAddTerms.push_back(factored);

    if (finalAddTerms.size() == 1)
        return finalAddTerms[0];

    return store->create(OpType::Add, std::move(finalAddTerms), bitWidth).id;
}

static ExprId Rewrite_CommonFactorCancel_PowTerms(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const std::vector<ExprId> lhsInputs = lhs.inputs;
    const std::vector<ExprId> rhsInputs = rhs.inputs;

    std::vector<bool> lhsConsumed(lhsInputs.size(), false);
    std::vector<bool> rhsConsumed(rhsInputs.size(), false);

    bool anyCanceled = false;

    for (size_t i = 0; i < lhsInputs.size(); ++i) {
        const ExprId lhsFactorId = lhsInputs[i];
        const Expr& lhsFactor = (*store)[lhsFactorId];
        if (lhsFactor.op != OpType::Pow || lhsFactor.inputs.size() != 2)
            continue;

        const Expr& lhsExp = (*store)[lhsFactor.inputs[1]];
        if (lhsExp.op != OpType::Const)
            continue;

        for (size_t j = 0; j < rhsInputs.size(); ++j) {
            if (rhsConsumed[j])
                continue;

            const ExprId rhsFactorId = rhsInputs[j];
            const Expr& rhsFactor = (*store)[rhsFactorId];
            if (rhsFactor.op != OpType::Pow || rhsFactor.inputs.size() != 2)
                continue;

            if (lhsFactor.inputs[0] != rhsFactor.inputs[0])
                continue;

            const Expr& rhsExp = (*store)[rhsFactor.inputs[1]];
            if (rhsExp.op != OpType::Const || lhsExp.knownValue != rhsExp.knownValue)
                continue;

            lhsConsumed[i] = true;
            rhsConsumed[j] = true;
            anyCanceled = true;
            break;
        }
    }

    if (!anyCanceled)
        return id;

    auto buildProduct = [&](const std::vector<ExprId> mulExprInputs, const std::vector<bool>& consumed) -> ExprId {
        std::vector<ExprId> remaining;
        remaining.reserve(mulExprInputs.size());

        for (size_t k = 0; k < mulExprInputs.size(); ++k) {
            if (!consumed[k])
                remaining.push_back(mulExprInputs[k]);
        }

        if (remaining.empty())
            return store->createConstant(1, bitWidth).id;
        if (remaining.size() == 1)
            return remaining[0];
        return store->create(OpType::Mul, std::move(remaining), bitWidth).id;
    };

    const ExprId newLhs = buildProduct(lhsInputs, lhsConsumed);
    const ExprId newRhs = buildProduct(rhsInputs, rhsConsumed);

    return store->create(OpType::Div, {newLhs, newRhs}, bitWidth).id;
}
#pragma endregion

Rule Get_AddLinearMultiplicity_Rule() {
    return Rule{
        AddLinearMultiplicity, &Match_AddLinearMultiplicity, &Rewrite_AddLinearMultiplicity, {Normalize::Order}};
}

Rule Get_AddCommonFactor_Rule() {
    return Rule{AddCommonFactor, &Match_AddCommonFactor, &Rewrite_AddCommonFactor, {AddLinearMultiplicity}};
}

Rule Get_CommonFactorCancel_PowTerms_Rule() {
    return Rule{CommonFactorCancel_PowTerms,
                &Match_CommonFactorCancel_PowTerms,
                &Rewrite_CommonFactorCancel_PowTerms,
                {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
