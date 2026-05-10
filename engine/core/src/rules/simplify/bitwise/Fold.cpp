#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_And_Fold(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}

static bool Match_Or_Fold(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}

static bool Match_Xor_Fold(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const)
            constCount++;
    }

    return constCount >= 2;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_And_Fold(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && store->isFalse(in))
            return store->makeFalse(e.bitWidth).id;

        if (exprIn.op == OpType::Const && store->isTrue(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeTrue(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Or_Fold(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && store->isTrue(in))
            return store->makeTrue(e.bitWidth).id;

        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Xor_Fold(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = store->get(inId);

        if (in.op == OpType::Const) {
            acc ^= in.knownValue;
            hasConst = true;
        } else {
            nonConst.push_back(inId);
        }
    }

    acc &= mask;

    if (!hasConst)
        return id;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Xor, std::move(nonConst), e.bitWidth).id;
}
#pragma endregion

Rule Get_And_Fold_Rule() {
    return Rule{And_Fold, &Match_And_Fold, &Rewrite_And_Fold, {Normalize::Flatten}};
}

Rule Get_Or_Fold_Rule() {
    return Rule{Or_Fold, &Match_Or_Fold, &Rewrite_Or_Fold, {Normalize::Flatten}};
}

Rule Get_Xor_Fold_Rule() {
    return Rule{Xor_Fold, &Match_Xor_Fold, &Rewrite_Xor_Fold, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
