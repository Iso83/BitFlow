#include "RuleOrderException.h"
#include "ast/ExprIntern.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

using Expr = AST::Expr;

void RuleEngine::AddRule(const Rule& rule) {
#ifndef NDEBUG
    if (!m_rules.empty()) {
        const Rule& last = m_rules.back();

        if (rule.stage < last.stage)
            throw RuleOrderException("Rule stage regression");
    }
#endif

    if (!ValidateRule(rule))
        throw std::runtime_error("Rule validation failed");

    for (RuleId dep : rule.deps) {
        if (!HasRule(dep)) {
            throw std::runtime_error("Missing rule dependency");
        }
    }

    m_rules.push_back(rule);
    m_present.insert(rule.id);
}

Expr* RuleEngine::ApplyOnce(Expr* expr) const {
    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& r : m_rules) {
            if (r.match(*expr)) {
                Expr* next = r.rewrite(*expr);

                if (next != expr) {
                    expr = next;
                    changed = true;
                    break;
                }
            }
        }
    }

    return AST::ExprIntern::Intern(expr);
}

Expr* RuleEngine::ApplyRecursive(Expr* expr) const {
    bool changed = false;
    std::vector<Expr*> newInputs;
    newInputs.reserve(expr->inputs.size());

    for (Expr* input : expr->inputs) {
        Expr* rewritten = ApplyRecursive(input);
        newInputs.push_back(rewritten);

        if (rewritten != input)
            changed = true;
    }

    Expr* current = expr;

    if (changed) {
        Expr* n = new Expr{};
        n->op = expr->op;
        n->isConst = expr->isConst;
        n->constValue = expr->constValue;
        n->inputs = std::move(newInputs);

        current = n;
    }

    return ApplyOnce(current);
}

Expr* RuleEngine::ApplyUntilStable(Expr* expr) const {
    while (true) {
        Expr* next = ApplyRecursive(expr);

        if (next == expr)
            return expr;

        expr = next;
    }
}

} // namespace BitFlow::Core::Rules