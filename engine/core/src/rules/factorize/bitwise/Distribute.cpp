#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Helpers
static ExprId BuildAnd(ExprStore* store, ExprId a, ExprId b) {
    const Expr& exprA = store->get(a);
    if (exprA.op == OpType::Const && IsFalse(store, a))
        return a;

    const Expr& exprB = store->get(b);
    if (exprB.op == OpType::Const && IsFalse(store, b))
        return b;

    if (exprA.op == OpType::Const && IsTrue(store, a))
        return b;

    if (exprB.op == OpType::Const && IsTrue(store, b))
        return a;

    return store->create(OpType::And, {a, b}).id;
}
#pragma endregion

static bool Match_Distribute_And_Over_Xor(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op != OpType::Xor || exprIn.inputs.size() < 2)
            continue;

        const size_t termCount = exprIn.inputs.size();
        const size_t otherCount = e.inputs.size() - 1;

        if (termCount * (otherCount + 1) > 12)
            continue;

        return true;
    }

    return false;
}

static ExprId Rewrite_Distribute_And_Over_Xor(ExprStore* store, ExprId id) {
    const auto& e = store->get(id);

    ExprId xorNodeID;
    Expr* xorNode = nullptr;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(id);
        if (exprIn.op == OpType::Xor && exprIn.inputs.size() >= 2) {
            xorNodeID = in;
            xorNode = &store->get(in);
            break;
        }
    }

    if (xorNode == nullptr) {
        _ASSERT(false);
        return id;
    }

    std::vector<ExprId> others;
    others.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        if (in != xorNodeID)
            others.push_back(in);
    }

    std::vector<ExprId> distributed;
    distributed.reserve(xorNode->inputs.size());

    for (auto term : xorNode->inputs) {
        ExprId acc = term;

        for (auto other : others)
            acc = BuildAnd(store, acc, other);

        distributed.push_back(acc);
    }

    if (distributed.empty())
        return store->makeFalse(e.bitWidth).id;

    if (distributed.size() == 1)
        return distributed[0];

    return store->create(OpType::Xor, std::move(distributed)).id;
}

Rule Get_Distribute_Rule() {
    return Rule{Distribute, &Match_Distribute_And_Over_Xor, &Rewrite_Distribute_And_Over_Xor, {Normalize::Flatten}};
}
} // namespace BitFlow::Core::Rules::Factorize::Bitwise
