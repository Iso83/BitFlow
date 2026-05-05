#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_Not(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = store->get(e.inputs[0]);

    if (in.op == OpType::Not && in.inputs.size() == 1)
        return true;

    if (in.op == OpType::Const)
        return true;

    return false;
}

static bool Match_NotPushdown(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = store->get(e.inputs[0]);

    if (!(in.op == OpType::And || in.op == OpType::Or))
        return false;

    bool allNot = true;
    for (auto child : in.inputs) {
        const Expr& exprChild = store->get(child);
        if (exprChild.op != OpType::Not) {
            allNot = false;
            break;
        }
    }

    return !allNot;
}

static bool Match_Not_Xor(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr& in = store->get(e.inputs[0]);

    return (in.op == OpType::Xor && in.inputs.size() >= 1);
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_Not(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    ExprId in = e.inputs[0];
    const Expr& exprIn = store->get(in);

    if (exprIn.op == OpType::Not && exprIn.inputs.size() == 1)
        return exprIn.inputs[0];

    if (exprIn.op == OpType::Const)
        return store->invertConst(in).id;

    _ASSERT(false);
    return id;
}

static ExprId Rewrite_NotPushdown(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    ExprId in = e.inputs[0];
    const Expr& exprIn = store->get(in);

    OpType newOp = (exprIn.op == OpType::And) ? OpType::Or : OpType::And;

    std::vector<ExprId> newInputs;
    newInputs.reserve(exprIn.inputs.size());

    for (auto child : exprIn.inputs)
        newInputs.push_back(store->create(OpType::Not, {child}, e.bitWidth).id);

    return store->create(newOp, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Not_Xor(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    ExprId in = e.inputs[0];
    const Expr& exprIn = store->get(in);

    std::vector<ExprId> newInputs;
    newInputs.reserve(exprIn.inputs.size() + 1);

    for (auto child : exprIn.inputs)
        newInputs.push_back(child);

    newInputs.push_back(store->makeTrue(e.bitWidth).id);

    return store->create(OpType::Xor, std::move(newInputs), e.bitWidth).id;
}
#pragma endregion

Rule Get_Not_Rule() {
    return Rule{Not, &Match_Not, &Rewrite_Not};
}

Rule Get_Not_Pushdown_Rule() {
    return Rule{Not_Pushdown, &Match_NotPushdown, &Rewrite_NotPushdown};
}

Rule Get_Not_Xor_Rule() {
    return Rule{Not_Xor, &Match_Not_Xor, &Rewrite_Not_Xor, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
