#include "expression/ExprClone.h"
#include "rules/RuleStage.h"
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

#pragma region Match
static bool Match_And_Fold(const Expr& e) {
    if (e.op != AST::OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}

static bool Match_Or_Fold(const Expr& e) {
    if (e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}

static bool Match_Xor_Fold(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (const Expr* in : e.inputs) {
        if (in->isConst)
            constCount++;
    }

    return constCount >= 2;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Fold(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (in->isConst && in->constValue == 0)
            return Expression::ConstPool::Get(0);

        if (in->isConst && in->constValue == 1)
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(1);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Or_Fold(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (in->isConst && in->constValue == 1)
            return Expression::ConstPool::Get(1);

        if (in->isConst && in->constValue == 0)
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Xor_Fold(Expr& e) {
    uint32_t acc = 0;
    std::vector<Expr*> nonConst;

    for (Expr* in : e.inputs) {
        if (in->isConst)
            acc ^= in->constValue;
        else
            nonConst.push_back(in);
    }

    if (acc != 0) {
        Expr* c = Expression::ConstPool::Get(acc);
        nonConst.push_back(c);
    }

    if (nonConst.empty()) {
        Expr* c = new Expr{};
        c->isConst = true;
        c->constValue = acc;
        c->id = Ids::ExprId{999998};
        return c;
    }

    if (nonConst.size() == 1)
        return nonConst[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(nonConst);
    return target;
}
#pragma endregion

Rule Get_Simplify_And_Fold_Rule() {
    return Rule{&Match_And_Fold, &Rewrite_And_Fold, Stage_Simplify};
}

Rule Get_Simplify_Or_Fold_Rule() {
    return Rule{&Match_Or_Fold, &Rewrite_Or_Fold, Stage_Simplify};
}

Rule Get_Simplify_Xor_Fold_Rule() {
    return Rule{&Match_Xor_Fold, &Rewrite_Xor_Fold, Stage_Simplify};
}

} // namespace BitFlow::Core::Rules::Simplify