#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RewriteCost.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Helpers

static Expr* BuildAnd(Expr* a, Expr* b) {
    if (a->isConst() && a->constValue == 0)
        return a;

    if (b->isConst() && b->constValue == 0)
        return b;

    if (a->isConst() && a->constValue == 1)
        return b;

    if (b->isConst() && b->constValue == 1)
        return a;

    auto* n = Expression::MakeOpInterned(OpType::And, {a, b});
    return n;
}

static Expr* BuildXor(const std::vector<Expr*>& inputs) {
    if (inputs.empty())
        return Expression::ConstPool::Get(0);

    if (inputs.size() == 1)
        return inputs[0];

    Expr* n = Expression::MakeOpInterned(OpType::Xor, inputs);
    return n;
}

#pragma endregion

#pragma region Match

static bool Match_Distribute_And_Over_Xor(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op != OpType::Xor || in->inputs.size() < 2)
            continue;

        const size_t termCount = in->inputs.size();
        const size_t otherCount = e.inputs.size() - 1;

        // beperkte distributie: voorkom explosie
        if (termCount * (otherCount + 1) > 12)
            continue;

        return true;
    }

    return false;
}

#pragma endregion

#pragma region Rewrite

static Expr* Rewrite_Distribute_And_Over_Xor(Expr& e) {
    Expr* xorNode = nullptr;

    for (Expr* in : e.inputs) {
        if (in->op == OpType::Xor && in->inputs.size() >= 2) {
            xorNode = in;
            break;
        }
    }

    if (xorNode == nullptr)
        return nullptr;

    std::vector<Expr*> others;
    others.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (in != xorNode)
            others.push_back(in);
    }

    std::vector<Expr*> distributed;
    distributed.reserve(xorNode->inputs.size());

    for (Expr* term : xorNode->inputs) {
        Expr* acc = term;

        for (Expr* other : others)
            acc = BuildAnd(acc, other);

        distributed.push_back(acc);
    }

    if (distributed.empty())
        return Expression::ConstPool::Get(0);

    if (distributed.size() == 1)
        return distributed[0];

    auto* n = Expression::MakeOpInterned(OpType::Xor, std::move(distributed));
    if (!IsRewritePreferred(n, &e, RewriteCostPolicy::ExpandDistribute))
        return nullptr;
    return n;
}

#pragma endregion

Rule Get_Distribute_Rule() {
    return Rule{RuleId::Factorize_Distribute, &Match_Distribute_And_Over_Xor, &Rewrite_Distribute_And_Over_Xor, Stage_Factorize, {RuleId::Normalize_Flatten}, RuleFlags::Factorizing | RuleFlags::Expanding, "Factorize_Distribute"};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise
