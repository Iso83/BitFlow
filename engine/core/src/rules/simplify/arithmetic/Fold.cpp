#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_AddFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Add)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const)
            constCount++;
    }

    if (constCount >= 2)
        return true;

    for (std::size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& lhs = (*store)[e.inputs[i]];
        if (lhs.op != OpType::Div || lhs.inputs.size() != 2)
            continue;

        const Expr& lhsNum = (*store)[lhs.inputs[0]];
        const Expr& lhsDen = (*store)[lhs.inputs[1]];
        if (lhsNum.op != OpType::Const || lhsDen.op != OpType::Const || lhsDen.knownValue == 0)
            continue;

        for (std::size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr& rhs = (*store)[e.inputs[j]];
            if (rhs.op != OpType::Div || rhs.inputs.size() != 2)
                continue;

            const Expr& rhsNum = (*store)[rhs.inputs[0]];
            const Expr& rhsDen = (*store)[rhs.inputs[1]];
            if (rhsNum.op == OpType::Const && rhsDen.op == OpType::Const && lhsDen.knownValue == rhsDen.knownValue)
                return true;
        }
    }

    return false;
}

static bool Match_SubConstFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Sub || e.inputs.size() < 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    if (lhs.op != OpType::Add)
        return false;

    if (lhs.inputs.size() < 2)
        return false;

    if (rhs.op != OpType::Const)
        return false;

    int constCount = 0;

    for (ExprId in : lhs.inputs) {
        if ((*store)[in].op == OpType::Const)
            constCount++;
    }

    return constCount >= 1;
}

static bool Match_SubAddSelfCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() < 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    if (lhs.op != OpType::Add)
        return false;

    ExprInputs positives;
    ExprInputs negatives;
    auto collectTerms = [&](auto&& self, ExprId exprId, bool positive) -> void {
        const Expr& expr = (*store)[exprId];
        if (expr.op == OpType::Add) {
            for (ExprId inId : expr.inputs)
                self(self, inId, positive);
            return;
        }
        if (expr.op == OpType::Sub && expr.inputs.size() == 2) {
            self(self, expr.inputs[0], positive);
            self(self, expr.inputs[1], !positive);
            return;
        }
        if (positive)
            positives.push_back(exprId);
        else
            negatives.push_back(exprId);
    };

    collectTerms(collectTerms, e.inputs[0], true);
    collectTerms(collectTerms, e.inputs[1], false);

    for (ExprId posId : positives) {
        for (ExprId negId : negatives) {
            if (CompareExprCanonical(store, posId, negId) == 0)
                return true;
        }
    }
    return false;
}

static bool Match_SubMulLinearCancel(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() < 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    if (lhs.op != OpType::Mul || lhs.inputs.size() < 2)
        return false;

    auto decomposeMulLinear = [&](ExprId exprId, ExprId& baseOut, Types::ExprChunk& coeffOut) -> bool {
        const Expr& expr = (*store)[exprId];
        if (expr.op != OpType::Mul || expr.inputs.size() < 2)
            return false;

        bool hasBase = false;
        bool hasConst = false;
        coeffOut = 1;
        for (ExprId inId : expr.inputs) {
            const Expr& in = (*store)[inId];
            if (in.op == OpType::Const) {
                coeffOut *= in.knownValue;
                hasConst = true;
            } else if (!hasBase) {
                baseOut = inId;
                hasBase = true;
            } else
                return false;
        }
        return hasBase && hasConst;
    };

    ExprId lhsBase{};
    Types::ExprChunk lhsCoeff{};
    if (!decomposeMulLinear(e.inputs[0], lhsBase, lhsCoeff))
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    if (rhs.op == OpType::Mul) {
        ExprId rhsBase{};
        Types::ExprChunk rhsCoeff{};
        return decomposeMulLinear(e.inputs[1], rhsBase, rhsCoeff) && CompareExprCanonical(store, lhsBase, rhsBase) == 0;
    }

    return CompareExprCanonical(store, lhsBase, e.inputs[1]) == 0;
}

static bool Match_MulDivConstantReduction(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Div || e.inputs.size() < 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];
    if (lhs.op != OpType::Mul || lhs.inputs.size() < 2)
        return false;
    if (rhs.op != OpType::Const || rhs.knownValue == 0)
        return false;

    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];
        if (in.op == OpType::Const && (in.knownValue % rhs.knownValue) == 0)
            return true;
    }

    return false;
}

static bool Match_MulToPow(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Mul || e.inputs.size() < 2)
        return false;

    for (std::size_t i = 0; i < e.inputs.size(); ++i) {
        for (std::size_t j = i + 1; j < e.inputs.size(); ++j) {
            if (store->structuralEquivalent(e.inputs[i], e.inputs[j]))
                return true;
        }
    }

    return false;
}

static bool Match_MulPowCombine(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Mul || e.inputs.size() < 2)
        return false;

    for (std::size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& a = (*store)[e.inputs[i]];

        for (std::size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr& b = (*store)[e.inputs[j]];

            const ExprId aId = (a.op == OpType::Neg && a.inputs.size() == 1) ? a.inputs[0] : e.inputs[i];
            const ExprId bId = (b.op == OpType::Neg && b.inputs.size() == 1) ? b.inputs[0] : e.inputs[j];

            const Expr& aa = (*store)[aId];
            const Expr& bb = (*store)[bId];

            // x * pow(x, a)
            if (bb.op == OpType::Pow && bb.inputs.size() == 2) {
                if (store->structuralEquivalent(aId, bb.inputs[0]))
                    return true;
            }

            // pow(x, a) * x
            if (aa.op == OpType::Pow && aa.inputs.size() == 2) {
                if (store->structuralEquivalent(aa.inputs[0], bId))
                    return true;
            }

            // pow(x, a) * pow(x, b)
            if (aa.op == OpType::Pow && bb.op == OpType::Pow && aa.inputs.size() == 2 && bb.inputs.size() == 2) {

                if (store->structuralEquivalent(aa.inputs[0], bb.inputs[0]))
                    return true;
            }
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddFold(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const ExprInputs eInputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    // Combine fractions with the same constant denominator:
    // (a / d) + (b / d) -> (a + b) / d
    // and fold to constant when exact.
    for (std::size_t i = 0; i < eInputs.size(); ++i) {
        const Expr& lhs = (*store)[eInputs[i]];
        if (lhs.op != OpType::Div || lhs.inputs.size() != 2)
            continue;

        const Expr& lhsNum = (*store)[lhs.inputs[0]];
        const Expr& lhsDen = (*store)[lhs.inputs[1]];
        if (lhsNum.op != OpType::Const || lhsDen.op != OpType::Const || lhsDen.knownValue == 0)
            continue;

        for (std::size_t j = i + 1; j < eInputs.size(); ++j) {
            const Expr& rhs = (*store)[eInputs[j]];
            if (rhs.op != OpType::Div || rhs.inputs.size() != 2)
                continue;

            const Expr& rhsNum = (*store)[rhs.inputs[0]];
            const Expr& rhsDen = (*store)[rhs.inputs[1]];
            if (rhsNum.op != OpType::Const || rhsDen.op != OpType::Const)
                continue;
            if (lhsDen.knownValue != rhsDen.knownValue)
                continue;

            const Types::ExprChunk den = lhsDen.knownValue;
            const Types::ExprChunk sumNum = (lhsNum.knownValue + rhsNum.knownValue) & mask;

            ExprId combined =
                store
                    ->create(OpType::Div,
                             {store->createConstant(sumNum, bitWidth).id, store->createConstant(den, bitWidth).id},
                             bitWidth)
                    .id;
            if (den != 0 && (sumNum % den) == 0)
                combined = store->createConstant((sumNum / den) & mask, bitWidth).id;

            ExprInputs newInputs;
            newInputs.reserve(eInputs.size() - 1);
            for (std::size_t k = 0; k < eInputs.size(); ++k) {
                if (k == i || k == j)
                    continue;
                newInputs.push_back(eInputs[k]);
            }
            newInputs.push_back(combined);

            if (newInputs.size() == 1)
                return newInputs[0];
            return store->create(OpType::Add, std::move(newInputs), bitWidth).id;
        }
    }

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    ExprInputs nonConst;
    nonConst.reserve(eInputs.size());

    for (ExprId inId : eInputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            hasConst = true;
        } else
            nonConst.push_back(inId);
    }

    if (!hasConst)
        return id;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Add, std::move(nonConst), bitWidth).id;
}

static ExprId Rewrite_SubConstFold(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk lhsConst = 0;

    ExprInputs newInputs;
    newInputs.reserve(lhs.inputs.size());

    bool changed = false;

    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            lhsConst = (lhsConst + in.knownValue) & mask;
            changed = true;
        } else
            newInputs.push_back(inId);
    }

    if (!changed)
        return id;

    const Types::ExprChunk delta = (lhsConst - rhs.knownValue) & mask;

    if (newInputs.empty())
        return store->createConstant(delta, bitWidth).id;

    auto makeNonConst = [&]() -> ExprId {
        if (newInputs.size() == 1)
            return newInputs[0];

        return store->create(OpType::Add, std::move(newInputs), bitWidth).id;
    };

    if (delta == 0)
        return makeNonConst();

    const Types::ExprChunk positiveLimit = mask >> 1;

    if (delta <= positiveLimit) {
        newInputs.push_back(store->createConstant(delta, bitWidth).id);
        return store->create(OpType::Add, std::move(newInputs), bitWidth).id;
    }

    const ExprId nonConst = makeNonConst();
    const Types::ExprChunk subtrahend = (rhs.knownValue - lhsConst) & mask;

    return store->create(OpType::Sub, {nonConst, store->createConstant(subtrahend, bitWidth).id}, bitWidth).id;
}

static ExprId Rewrite_SubAddSelfCancel(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;

    ExprInputs positives;
    ExprInputs negatives;
    auto collectTerms = [&](auto&& self, ExprId exprId, bool positive) -> void {
        const Expr& expr = (*store)[exprId];
        if (expr.op == OpType::Add) {
            for (ExprId inId : expr.inputs)
                self(self, inId, positive);
            return;
        }
        if (expr.op == OpType::Sub && expr.inputs.size() == 2) {
            self(self, expr.inputs[0], positive);
            self(self, expr.inputs[1], !positive);
            return;
        }
        if (positive)
            positives.push_back(exprId);
        else
            negatives.push_back(exprId);
    };
    collectTerms(collectTerms, e.inputs[0], true);
    collectTerms(collectTerms, e.inputs[1], false);

    bool changed = false;
    std::vector<bool> negUsed(negatives.size(), false);
    ExprInputs remainingPositives;
    for (ExprId posId : positives) {
        bool matched = false;
        for (std::size_t i = 0; i < negatives.size(); ++i) {
            if (!negUsed[i] && CompareExprCanonical(store, posId, negatives[i]) == 0) {
                negUsed[i] = true;
                changed = true;
                matched = true;
                break;
            }
        }
        if (!matched)
            remainingPositives.push_back(posId);
    }

    ExprInputs remainingNegatives;
    for (std::size_t i = 0; i < negatives.size(); ++i) {
        if (!negUsed[i])
            remainingNegatives.push_back(negatives[i]);
    }

    if (!changed)
        return id;

    auto buildAdd = [&](ExprInputs& terms) -> ExprId {
        if (terms.empty())
            return store->createConstant(0, bitWidth).id;
        if (terms.size() == 1)
            return terms[0];
        return store->create(OpType::Add, std::move(terms), bitWidth).id;
    };

    ExprId positiveExpr = buildAdd(remainingPositives);

    if (remainingNegatives.empty())
        return positiveExpr;

    ExprId negativeExpr = buildAdd(remainingNegatives);

    return store->create(OpType::Sub, {positiveExpr, negativeExpr}, bitWidth).id;
}

static ExprId Rewrite_SubMulLinearCancel(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    const Expr& lhs = (*store)[e.inputs[0]];
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    auto decomposeMulLinear = [&](ExprId exprId, ExprId& baseOut, Types::ExprChunk& coeffOut) -> bool {
        const Expr& expr = (*store)[exprId];
        if (expr.op != OpType::Mul || expr.inputs.size() < 2)
            return false;
        bool hasBase = false;
        bool hasConst = false;
        coeffOut = 1;
        for (ExprId inId : expr.inputs) {
            const Expr& in = (*store)[inId];
            if (in.op == OpType::Const) {
                coeffOut = (coeffOut * in.knownValue) & mask;
                hasConst = true;
            } else if (!hasBase) {
                baseOut = inId;
                hasBase = true;
            } else
                return false;
        }
        return hasBase && hasConst;
    };

    ExprId lhsBase{};
    Types::ExprChunk lhsCoeff{};
    if (!decomposeMulLinear(e.inputs[0], lhsBase, lhsCoeff))
        return id;

    Types::ExprChunk rhsCoeff = 1;
    const Expr& rhs = (*store)[e.inputs[1]];
    if (rhs.op == OpType::Mul) {
        ExprId rhsBase{};
        if (!decomposeMulLinear(e.inputs[1], rhsBase, rhsCoeff) || CompareExprCanonical(store, lhsBase, rhsBase) != 0)
            return id;
    } else if (CompareExprCanonical(store, lhsBase, e.inputs[1]) != 0)
        return id;

    Types::ExprChunk coeff = (lhsCoeff - rhsCoeff) & mask;
    if (coeff == 0)
        return store->zeroId();
    if (coeff == mask)
        return store->create(OpType::Neg, {lhsBase}, bitWidth).id;
    if (coeff == 1)
        return lhsBase;

    const ExprId coeffId = store->createConstant(coeff, bitWidth).id;
    ExprInputs mulInputs{lhsBase, coeffId};

    if (mulInputs.size() == 1)
        return mulInputs[0];
    return store->create(OpType::Mul, std::move(mulInputs), bitWidth).id;
}

static ExprId Rewrite_MulDivConstantReduction(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];
    const Types::BitWidth bitWidth = e.bitWidth;

    ExprInputs newMulInputs;
    newMulInputs.reserve(lhs.inputs.size());

    bool reduced = false;
    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];
        if (!reduced && in.op == OpType::Const && rhs.knownValue != 0 && (in.knownValue % rhs.knownValue) == 0) {
            const Types::ExprChunk reducedConst = (in.knownValue / rhs.knownValue) & Expr::fullMask(bitWidth);
            if (reducedConst != 1)
                newMulInputs.push_back(store->createConstant(reducedConst, bitWidth).id);
            reduced = true;
            continue;
        }
        newMulInputs.push_back(inId);
    }

    if (!reduced)
        return id;

    if (newMulInputs.empty())
        return store->createConstant(1, bitWidth).id;
    if (newMulInputs.size() == 1)
        return newMulInputs[0];
    return store->create(OpType::Mul, std::move(newMulInputs), bitWidth).id;
}

static ExprId Rewrite_MulToPow(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const ExprInputs inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;
    std::vector<bool> consumed(e.inputs.size(), false);
    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (std::size_t i = 0; i < inputs.size(); ++i) {
        if (consumed[i])
            continue;

        std::size_t multiplicity = 1;
        consumed[i] = true;

        for (std::size_t j = i + 1; j < inputs.size(); ++j) {
            if (!consumed[j] && CompareExprCanonical(store, inputs[i], inputs[j]) == 0) {
                consumed[j] = true;
                ++multiplicity;
            }
        }

        if (multiplicity >= 2) {
            const ExprId exp = store->createConstant(static_cast<Types::ExprChunk>(multiplicity), bitWidth).id;
            newInputs.push_back(store->create(OpType::Pow, {inputs[i], exp}, bitWidth).id);
        } else
            newInputs.push_back(inputs[i]);
    }

    if (newInputs.empty())
        return store->createConstant(1, bitWidth).id;
    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Mul, std::move(newInputs), bitWidth).id;
}

static ExprId Rewrite_MulPowCombine(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const ExprInputs inputs = e.inputs;
    const Types::BitWidth bw = e.bitWidth;
    ExprId expAId = store->createConstant(1, bw).id;
    ExprId expBId = store->createConstant(1, bw).id;

    std::vector<bool> consumed(inputs.size(), false);
    ExprInputs newInputs;
    bool negateResult = false;

    for (std::size_t i = 0; i < inputs.size(); ++i) {
        if (consumed[i])
            continue;

        const Expr& a = (*store)[inputs[i]];
        bool combined = false;

        for (std::size_t j = i + 1; j < inputs.size(); ++j) {
            if (consumed[j])
                continue;

            const Expr& b = (*store)[inputs[j]];

            ExprId baseId{};
            bool match = false;

            const bool aNeg = (a.op == OpType::Neg && a.inputs.size() == 1);
            const bool bNeg = (b.op == OpType::Neg && b.inputs.size() == 1);
            const ExprId aId = aNeg ? a.inputs[0] : inputs[i];
            const ExprId bId = bNeg ? b.inputs[0] : inputs[j];
            const Expr& aa = (*store)[aId];
            const Expr& bb = (*store)[bId];

            // x * pow(x, a)
            if (bb.op == OpType::Pow && bb.inputs.size() == 2 && store->structuralEquivalent(aId, bb.inputs[0])) {
                baseId = aId;
                expBId = bb.inputs[1];
                match = true;
            }

            // pow(x,a) * x
            else if (aa.op == OpType::Pow && aa.inputs.size() == 2 && store->structuralEquivalent(aa.inputs[0], bId)) {
                baseId = aa.inputs[0];
                expAId = aa.inputs[1];
                match = true;
            }

            // pow(x,a) * pow(x,b)
            else if (aa.op == OpType::Pow && bb.op == OpType::Pow && aa.inputs.size() == 2 && bb.inputs.size() == 2 &&
                     store->structuralEquivalent(aa.inputs[0], bb.inputs[0])) {

                baseId = aa.inputs[0];
                expAId = aa.inputs[1];
                expBId = bb.inputs[1];
                match = true;
            }

            if (!match)
                continue;

            consumed[i] = true;
            consumed[j] = true;
            negateResult ^= (aNeg ^ bNeg);

            const Expr& expA = (*store)[expAId];
            const Expr& expB = (*store)[expBId];
            ExprId expId{};
            if (expA.op == OpType::Const && expB.op == OpType::Const) {
                const Types::ExprChunk sum = (expA.knownValue + expB.knownValue) & Expr::fullMask(bw);
                expId = store->createConstant(sum, bw).id;
            } else {
                expId = store->create(OpType::Add, {expAId, expBId}, bw).id;
            }

            newInputs.push_back(store->create(OpType::Pow, {baseId, expId}, bw).id);

            combined = true;
            break;
        }

        if (!combined && !consumed[i]) {
            consumed[i] = true;
            newInputs.push_back(inputs[i]);
        }
    }

    if (newInputs.empty())
        return store->createConstant(1, bw).id;

    for (std::size_t i = 0; i < inputs.size(); ++i) {
        if (consumed[i])
            continue;

        const Expr& in = (*store)[inputs[i]];
        if (in.op == OpType::Neg && in.inputs.size() == 1) {
            negateResult = !negateResult;
            newInputs.push_back(in.inputs[0]);
        } else
            newInputs.push_back(inputs[i]);
    }

    if (newInputs.size() == 1)
        return negateResult ? store->create(OpType::Neg, {newInputs[0]}, bw).id : newInputs[0];

    ExprId result = store->create(OpType::Mul, std::move(newInputs), bw).id;
    if (negateResult)
        return store->create(OpType::Neg, {result}, bw).id;
    return result;
}
#pragma endregion

Rule Get_AddFold_Rule() {
    return Rule{AddFold, &Match_AddFold, &Rewrite_AddFold, {Normalize::Flatten}};
}

Rule Get_SubConstFold_Rule() {
    return Rule{SubConstFold, &Match_SubConstFold, &Rewrite_SubConstFold, {Simplify::Arithmetic::AddFold}};
}

Rule Get_SubAddSelfCancel_Rule() {
    return Rule{SubAddSelfCancel,
                &Match_SubAddSelfCancel,
                &Rewrite_SubAddSelfCancel,
                {Normalize::Order, Simplify::Arithmetic::AddFold}};
}

Rule Get_SubMulLinearCancel_Rule() {
    return Rule{SubMulLinearCancel, &Match_SubMulLinearCancel, &Rewrite_SubMulLinearCancel, {Normalize::Order}};
}

Rule Get_MulDivConstantReduction_Rule() {
    return Rule{MulDivConstantReduction,
                &Match_MulDivConstantReduction,
                &Rewrite_MulDivConstantReduction,
                {Normalize::Flatten}};
}

Rule Get_MulToPow_Rule() {
    return Rule{MulToPow, &Match_MulToPow, &Rewrite_MulToPow, {CombineMulPow}};
}

Rule Get_CombineMulPow_Rule() {
    return Rule{CombineMulPow, &Match_MulPowCombine, &Rewrite_MulPowCombine, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
