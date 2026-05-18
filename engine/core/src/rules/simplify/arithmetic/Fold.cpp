#include "expression/ExprUtils.h"

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

    return constCount >= 2;
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
    if (lhs.op != OpType::Add || lhs.inputs.size() < 2)
        return false;

    const ExprId rhsId = e.inputs[1];
    for (ExprId inId : lhs.inputs) {
        if (CompareExprCanonical(store, inId, rhsId) == 0)
            return true;
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

    const ExprId rhsId = e.inputs[1];
    bool hasBase = false;
    bool hasConst = false;
    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];
        if (in.op == OpType::Const)
            hasConst = true;
        if (CompareExprCanonical(store, inId, rhsId) == 0)
            hasBase = true;
    }
    return hasBase && hasConst;
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
            if (CompareExprCanonical(store, e.inputs[i], e.inputs[j]) == 0)
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

            // x * pow(x, n)
            if (b.op == OpType::Pow && b.inputs.size() == 2) {
                const Expr& exp = (*store)[b.inputs[1]];
                if (exp.op == OpType::Const && CompareExprCanonical(store, e.inputs[i], b.inputs[0]) == 0)
                    return true;
            }

            // pow(x, n) * x
            if (a.op == OpType::Pow && a.inputs.size() == 2) {
                const Expr& exp = (*store)[a.inputs[1]];
                if (exp.op == OpType::Const && CompareExprCanonical(store, a.inputs[0], e.inputs[j]) == 0)
                    return true;
            }

            // pow(x, a) * pow(x, b)
            if (a.op == OpType::Pow && b.op == OpType::Pow && a.inputs.size() == 2 && b.inputs.size() == 2) {

                const Expr& expA = (*store)[a.inputs[1]];
                const Expr& expB = (*store)[b.inputs[1]];

                if (expA.op == OpType::Const && expB.op == OpType::Const &&
                    CompareExprCanonical(store, a.inputs[0], b.inputs[0]) == 0)
                    return true;
            }
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AddFold(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            hasConst = true;
        } else
            nonConst.push_back(inId);
    }

    if (!hasConst)
        return id;

    const Types::BitWidth bitWidth = e.bitWidth;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Add, std::move(nonConst), bitWidth).id;
}

static ExprId Rewrite_SubConstFold(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const Types::BitWidth bitWidth = e.bitWidth;
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk lhsConst = 0;

    std::vector<ExprId> newInputs;
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

static ExprId Rewrite_SubAddSelfCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Expr& lhs = (*store)[e.inputs[0]];
    const ExprId rhsId = e.inputs[1];

    std::vector<ExprId> newInputs;
    newInputs.reserve(lhs.inputs.size());

    bool removed = false;
    for (ExprId inId : lhs.inputs) {
        if (!removed && CompareExprCanonical(store, inId, rhsId) == 0) {
            removed = true;
            continue;
        }
        newInputs.push_back(inId);
    }

    if (!removed)
        return id;

    if (newInputs.empty())
        return store->createConstant(0, e.bitWidth).id;
    if (newInputs.size() == 1)
        return newInputs[0];
    return store->create(OpType::Add, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_SubMulLinearCancel(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    const Expr& lhs = (*store)[e.inputs[0]];
    const ExprId rhsId = e.inputs[1];
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk coeff = 1;
    bool removedBase = false;
    std::vector<ExprId> otherFactors;
    otherFactors.reserve(lhs.inputs.size());

    for (ExprId inId : lhs.inputs) {
        const Expr& in = (*store)[inId];
        if (!removedBase && CompareExprCanonical(store, inId, rhsId) == 0) {
            removedBase = true;
            continue;
        }

        if (in.op == OpType::Const)
            coeff = (coeff * in.knownValue) & mask;
        else
            otherFactors.push_back(inId);
    }

    if (!removedBase)
        return id;

    coeff = (coeff - 1) & mask;
    if (coeff == 0)
        return store->createConstant(0, bitWidth).id;
    if (coeff == 1)
        return rhsId;

    const ExprId coeffId = store->createConstant(coeff, bitWidth).id;
    std::vector<ExprId> mulInputs;
    mulInputs.reserve(otherFactors.size() + 2);
    mulInputs.push_back(rhsId);
    for (ExprId f : otherFactors)
        mulInputs.push_back(f);
    mulInputs.push_back(coeffId);

    if (mulInputs.size() == 1)
        return mulInputs[0];
    return store->create(OpType::Mul, std::move(mulInputs), bitWidth).id;
}

static ExprId Rewrite_MulDivConstantReduction(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];
    const Types::BitWidth bitWidth = e.bitWidth;

    std::vector<ExprId> newMulInputs;
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

static ExprId Rewrite_MulToPow(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const std::vector<ExprId> inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;
    std::vector<bool> consumed(e.inputs.size(), false);
    std::vector<ExprId> newInputs;
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

static ExprId Rewrite_MulPowCombine(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const std::vector<ExprId> inputs = e.inputs;
    const Types::BitWidth bw = e.bitWidth;

    std::vector<bool> consumed(inputs.size(), false);
    std::vector<ExprId> newInputs;

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
            Types::ExprChunk expA = 1;
            Types::ExprChunk expB = 1;

            bool match = false;

            // x * pow(x, n)
            if (b.op == OpType::Pow && b.inputs.size() == 2 &&
                CompareExprCanonical(store, e.inputs[i], b.inputs[0]) == 0) {

                const Expr& exp = (*store)[b.inputs[1]];
                if (exp.op == OpType::Const) {
                    baseId = e.inputs[i];
                    expB = exp.knownValue;
                    match = true;
                }
            }

            // pow(x,n) * x
            else if (a.op == OpType::Pow && a.inputs.size() == 2 &&
                     CompareExprCanonical(store, a.inputs[0], e.inputs[j]) == 0) {

                const Expr& exp = (*store)[a.inputs[1]];
                if (exp.op == OpType::Const) {
                    baseId = a.inputs[0];
                    expA = exp.knownValue;
                    match = true;
                }
            }

            // pow(x,a) * pow(x,b)
            else if (a.op == OpType::Pow && b.op == OpType::Pow && a.inputs.size() == 2 && b.inputs.size() == 2 &&
                     CompareExprCanonical(store, a.inputs[0], b.inputs[0]) == 0) {

                const Expr& ea = (*store)[a.inputs[1]];
                const Expr& eb = (*store)[b.inputs[1]];

                if (ea.op == OpType::Const && eb.op == OpType::Const) {
                    baseId = a.inputs[0];
                    expA = ea.knownValue;
                    expB = eb.knownValue;
                    match = true;
                }
            }

            if (!match)
                continue;

            consumed[i] = true;
            consumed[j] = true;

            const Types::ExprChunk sum = (expA + expB) & Expr::fullMask(bw);

            const ExprId expId = store->createConstant(sum, bw).id;

            newInputs.push_back(store->create(OpType::Pow, {baseId, expId}, bw).id);

            combined = true;
            break;
        }

        if (!combined && !consumed[i]) {
            consumed[i] = true;
            newInputs.push_back(e.inputs[i]);
        }
    }

    if (newInputs.empty())
        return store->createConstant(1, bw).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(OpType::Mul, std::move(newInputs), bw).id;
}
#pragma endregion

Rule Get_AddFold_Rule() {
    return Rule{AddFold, &Match_AddFold, &Rewrite_AddFold, {Normalize::Flatten}};
}

Rule Get_SubConstFold_Rule() {
    return Rule{SubConstFold, &Match_SubConstFold, &Rewrite_SubConstFold, {Simplify::Arithmetic::AddFold}};
}

Rule Get_SubAddSelfCancel_Rule() {
    return Rule{SubAddSelfCancel, &Match_SubAddSelfCancel, &Rewrite_SubAddSelfCancel, {Normalize::Order}};
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
