#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize {

using Expr = AST::Expr;

#pragma region Match
static bool Match_And_Absorb(const Expr& e) {
    if (e.op != AST::OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != AST::OpType::Or)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id)
                    return true;
            }
        }
    }

    return false;
}

static bool Match_Or_Absorb(const Expr& e) {
    if (e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != AST::OpType::And)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id)
                    return true;
            }
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Absorb(Expr& e) {
    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != AST::OpType::Or)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id) {
                    return a;
                }
            }
        }
    }

    return &e;
}

static Expr* Rewrite_Or_Absorb(Expr& e) {
    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != AST::OpType::And)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id) {
                    return a;
                }
            }
        }
    }

    return &e;
}
#pragma endregion

Rule Get_And_Absorb_Rule() {
    return Rule{RuleId::Factorize_AndAbsorb,
                &Match_And_Absorb,
                &Rewrite_And_Absorb,
                Stage_Factorize,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Or_Absorb_Rule() {
    return Rule{
        RuleId::Factorize_OrAbsorb, &Match_Or_Absorb, &Rewrite_Or_Absorb, Stage_Factorize, {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize