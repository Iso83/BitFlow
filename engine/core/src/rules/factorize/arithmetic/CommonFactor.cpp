#include "expression/ExprUtils.h"

#include <BitFlow/core/expression/ExprRefUtils.h>
#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static void AddCoeff(std::unordered_map<ExprId, Types::ExprChunk>& coeffByBaseId, ExprInputs& baseOrder, ExprId base,
                     Types::ExprChunk coeff, Types::ExprChunk mask) {
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
                LinearTerm lhsLinear{};
                if (subRhs.op == OpType::Const &&
                    DecomposeLinearTerm(store, maybeSub.inputs[0], lhsLinear, e.bitWidth)) {
                    if (++baseTermCounts[lhsLinear.base] >= 2)
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

    const ExprInputs lhsFactors = lhs.op == OpType::Mul ? lhs.inputs : ExprInputs{e.inputs[0]};
    const ExprInputs rhsFactors = rhs.op == OpType::Mul ? rhs.inputs : ExprInputs{e.inputs[1]};

    for (ExprId lhsFactorId : lhsFactors) {
        const Expr& lhsFactor = (*store)[lhsFactorId];

        if (lhsFactor.op != OpType::Pow || lhsFactor.inputs.size() != 2)
            continue;

        const Expr& lhsExp = (*store)[lhsFactor.inputs[1]];
        if (lhsExp.op != OpType::Const)
            continue;

        for (ExprId rhsFactorId : rhsFactors) {
            const Expr& rhsFactor = (*store)[rhsFactorId];

            if (rhsFactor.op != OpType::Pow || rhsFactor.inputs.size() != 2)
                continue;

            if (!store->structuralEquivalent(lhsFactor.inputs[0], rhsFactor.inputs[0]))
                continue;

            const Expr& rhsExp = (*store)[rhsFactor.inputs[1]];
            if (rhsExp.op != OpType::Const)
                continue;

            // equal or reducible exponents
            if (lhsExp.knownValue == rhsExp.knownValue)
                return true;

            if (lhsExp.knownValue > rhsExp.knownValue)
                return true;

            if (rhsExp.knownValue > lhsExp.knownValue)
                return true;
        }
    }

    return false;
}

static bool Match_SubCommonDenominator(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    if (rhs.op != OpType::Div || rhs.inputs.size() != 2)
        return false;

    if (lhs.op == OpType::Div && lhs.inputs.size() == 2)
        return store->structuralEquivalent(lhs.inputs[1], rhs.inputs[1]);

    if (lhs.op != OpType::Add || lhs.inputs.size() < 2)
        return false;

    for (ExprId lhsTermId : lhs.inputs) {
        const Expr& lhsTerm = (*store)[lhsTermId];
        if (lhsTerm.op != OpType::Div || lhsTerm.inputs.size() != 2)
            continue;

        if (store->structuralEquivalent(lhsTerm.inputs[1], rhs.inputs[1]))
            return true;
    }

    return false;
}

static bool Match_AddCommonDenominator(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Add || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    if (lhs.op != OpType::Div || rhs.op != OpType::Div)
        return false;

    if (lhs.inputs.size() != 2 || rhs.inputs.size() != 2)
        return false;

    return store->structuralEquivalent(lhs.inputs[1], rhs.inputs[1]);
}

static bool Match_CommonFactorCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    if (rhs.op != OpType::Mul || rhs.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < rhs.inputs.size(); ++i) {
        const Expr& maybeDiv = (*store)[rhs.inputs[i]];
        if (maybeDiv.op != OpType::Div || maybeDiv.inputs.size() != 2)
            continue;

        for (size_t j = 0; j < rhs.inputs.size(); ++j) {
            if (i == j)
                continue;
            if (store->structuralEquivalent(maybeDiv.inputs[1], rhs.inputs[j]))
                return true;
        }
    }

    return false;
}

#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddLinearMultiplicity(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const ExprInputs originalInputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    std::unordered_map<ExprId, Types::ExprChunk> coeffByBaseId;

    ExprInputs baseOrder;
    baseOrder.reserve(originalInputs.size());

    ExprInputs passthroughTerms;
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
                LinearTerm lhsLinear{};
                if (subRhs.op == OpType::Const && DecomposeLinearTerm(store, term.inputs[0], lhsLinear, bitWidth)) {
                    AddCoeff(coeffByBaseId, baseOrder, lhsLinear.base, lhsLinear.coeff, mask);
                    pendingSubConst = (pendingSubConst + subRhs.knownValue) & mask;
                    continue;
                }
            }
            passthroughTerms.push_back(inputId);
            continue;
        }

        AddCoeff(coeffByBaseId, baseOrder, linear.base, linear.coeff, mask);
    }

    ExprInputs normalizedAddTerms;
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
        return ctx.replace(
            id,
            store->create(OpType::Sub, {addResult, store->createConstant(pendingSubConst, bitWidth).id}, bitWidth).id);

    return ctx.replace(id, addResult);
}

static ExprId Rewrite_AddCommonFactor(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
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

    ExprInputs sharedInnerTerms;
    ExprInputs untouchedTerms;

    sharedInnerTerms.reserve(e.inputs.size());
    untouchedTerms.reserve(e.inputs.size());

    for (ExprId termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op == OpType::Mul && term.inputs.size() >= 2) {
            bool hasCommonFactor = false;
            ExprInputs remainingFactors;
            remainingFactors.reserve(term.inputs.size());

            for (ExprId factorId : term.inputs) {
                if (!hasCommonFactor && store->structuralEquivalent(factorId, common)) {
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

    ExprInputs finalAddTerms;
    finalAddTerms.reserve(untouchedTerms.size() + 1);

    for (ExprId termId : untouchedTerms)
        finalAddTerms.push_back(termId);

    finalAddTerms.push_back(factored);

    if (finalAddTerms.size() == 1)
        return ctx.replace(id, finalAddTerms[0]);

    return ctx.replace(id, store->create(OpType::Add, std::move(finalAddTerms), bitWidth).id);
}

static ExprId Rewrite_CommonFactorCancel_PowTerms(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const Types::BitWidth bitWidth = e.bitWidth;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const ExprInputs lhsInputs = lhs.op == OpType::Mul ? lhs.inputs : ExprInputs{e.inputs[0]};
    const ExprInputs rhsInputs = rhs.op == OpType::Mul ? rhs.inputs : ExprInputs{e.inputs[1]};

    std::vector<bool> lhsConsumed(lhsInputs.size(), false);
    std::vector<bool> rhsConsumed(rhsInputs.size(), false);

    ExprInputs lhsExtraFactors;
    ExprInputs rhsExtraFactors;

    bool anyCanceled = false;

    for (size_t i = 0; i < lhsInputs.size(); ++i) {

        if (lhsConsumed[i])
            continue;

        const ExprId lhsFactorId = lhsInputs[i];
        const Expr& lhsFactor = (*store)[lhsFactorId];

        if (lhsFactor.op != OpType::Pow || lhsFactor.inputs.size() != 2)
            continue;

        const ExprId lhsBaseId = lhsFactor.inputs[0];

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

            if (!store->structuralEquivalent(lhsBaseId, rhsFactor.inputs[0]))
                continue;

            const Expr& rhsExp = (*store)[rhsFactor.inputs[1]];

            if (rhsExp.op != OpType::Const)
                continue;

            const Types::ExprChunk lhsValue = lhsExp.knownValue;
            const Types::ExprChunk rhsValue = rhsExp.knownValue;

            lhsConsumed[i] = true;
            rhsConsumed[j] = true;

            anyCanceled = true;

            // same exponent => fully cancel
            if (lhsValue == rhsValue)
                break;

            if (lhsValue > rhsValue) {

                const Types::ExprChunk diff = lhsValue - rhsValue;

                const ExprId diffId = store->createConstant(diff, bitWidth).id;

                lhsExtraFactors.push_back(store->create(OpType::Pow, {lhsBaseId, diffId}, bitWidth).id);

                break;
            }

            if (rhsValue > lhsValue) {

                const Types::ExprChunk diff = rhsValue - lhsValue;

                const ExprId diffId = store->createConstant(diff, bitWidth).id;

                rhsExtraFactors.push_back(store->create(OpType::Pow, {lhsBaseId, diffId}, bitWidth).id);

                break;
            }
        }
    }

    if (!anyCanceled)
        return id;

    auto buildProduct = [&](const ExprInputs& mulExprInputs, const std::vector<bool>& consumed,
                            const ExprInputs& extras) -> ExprId {
        ExprInputs remaining;

        remaining.reserve(mulExprInputs.size() + extras.size());

        for (size_t k = 0; k < mulExprInputs.size(); ++k) {
            if (!consumed[k])
                remaining.push_back(mulExprInputs[k]);
        }

        for (ExprId extraId : extras)
            remaining.push_back(extraId);

        if (remaining.empty())
            return ctx.replace(id, store->createConstant(1, bitWidth).id);

        if (remaining.size() == 1)
            return ctx.replace(id, remaining[0]);

        return ctx.replace(id, store->create(OpType::Mul, std::move(remaining), bitWidth).id);
    };

    const ExprId newLhs = buildProduct(lhsInputs, lhsConsumed, lhsExtraFactors);

    const ExprId newRhs = buildProduct(rhsInputs, rhsConsumed, rhsExtraFactors);

    // fully reduced
    if (EqualChunkValue(store, newRhs, 1u))
        return ctx.replace(id, newLhs);

    return ctx.replace(id, store->create(OpType::Div, {newLhs, newRhs}, bitWidth).id);
}

static ExprId Rewrite_SubCommonDenominator(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];
    const ExprId rhsDenominator = rhs.inputs[1];

    if (lhs.op == OpType::Div && lhs.inputs.size() == 2) {
        const ExprId numerator = store->create(OpType::Sub, {lhs.inputs[0], rhs.inputs[0]}, bitWidth).id;
        return ctx.replace(id, store->create(OpType::Div, {numerator, rhsDenominator}, bitWidth).id);
    }

    ExprInputs combinedAddTerms;
    combinedAddTerms.reserve(lhs.inputs.size());
    bool rewritten = false;

    for (ExprId lhsTermId : lhs.inputs) {
        const Expr& lhsTerm = (*store)[lhsTermId];

        if (!rewritten && lhsTerm.op == OpType::Div && lhsTerm.inputs.size() == 2 &&
            store->structuralEquivalent(lhsTerm.inputs[1], rhsDenominator)) {
            const ExprId numerator = store->create(OpType::Sub, {lhsTerm.inputs[0], rhs.inputs[0]}, bitWidth).id;
            const ExprId rewrittenDiv = store->create(OpType::Div, {numerator, rhsDenominator}, bitWidth).id;
            combinedAddTerms.push_back(rewrittenDiv);
            rewritten = true;
            continue;
        }

        combinedAddTerms.push_back(lhsTermId);
    }

    if (!rewritten)
        return id;

    return ctx.replace(id, store->create(OpType::Add, std::move(combinedAddTerms), bitWidth).id);
}

static ExprId Rewrite_AddCommonDenominator(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;

    const Expr& lhs = (*store)[e.inputs[0]];
    const ExprId denominator = lhs.inputs[1];

    const Expr& rhs = (*store)[e.inputs[1]];

    const ExprId numerator = store->create(OpType::Add, {lhs.inputs[0], rhs.inputs[0]}, bitWidth).id;
    return ctx.replace(id, store->create(OpType::Div, {numerator, denominator}, bitWidth).id);
}

static ExprId Rewrite_CommonFactorCancel(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    const ExprInputs eInputs = e.inputs;
    const Expr& rhs = (*store)[e.inputs[1]];

    for (size_t i = 0; i < rhs.inputs.size(); ++i) {
        const Expr& maybeDiv = (*store)[rhs.inputs[i]];
        if (maybeDiv.op != OpType::Div || maybeDiv.inputs.size() != 2)
            continue;

        for (size_t j = 0; j < rhs.inputs.size(); ++j) {
            if (i == j)
                continue;
            if (!store->structuralEquivalent(maybeDiv.inputs[1], rhs.inputs[j]))
                continue;

            ExprInputs newFactors;
            newFactors.reserve(rhs.inputs.size() - 1);
            for (size_t k = 0; k < rhs.inputs.size(); ++k) {
                if (k == i || k == j)
                    continue;
                newFactors.push_back(rhs.inputs[k]);
            }
            newFactors.push_back(maybeDiv.inputs[0]);

            ExprId newRhs = store->createConstant(1, bitWidth).id;
            if (!newFactors.empty())
                newRhs = (newFactors.size() == 1) ? newFactors[0]
                                                  : store->create(OpType::Mul, std::move(newFactors), bitWidth).id;

            return ctx.replace(id, store->create(OpType::Sub, {eInputs[0], newRhs}, bitWidth).id);
        }
    }

    return id;
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

Rule Get_SubCommonDenominator_Rule() {
    return Rule{SubCommonDenominator, &Match_SubCommonDenominator, &Rewrite_SubCommonDenominator, {Normalize::Order}};
}

Rule Get_AddCommonDenominator_Rule() {
    return Rule{AddCommonDenominator, &Match_AddCommonDenominator, &Rewrite_AddCommonDenominator, {Normalize::Order}};
}

Rule Get_CommonFactorCancel_Rule() {
    return Rule{CommonFactorCancel, &Match_CommonFactorCancel, &Rewrite_CommonFactorCancel, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
