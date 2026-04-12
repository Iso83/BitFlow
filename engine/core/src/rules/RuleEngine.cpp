#include "RuleOrderException.h"
#include "ast/ExprIntern.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <algorithm>

namespace BitFlow::Core::Rules {

using Expr = AST::Expr;

RuleEngine::RuleEngine() {
    AST::ExprIntern::Reset();
}

void RuleEngine::AddRule(const Rule& rule) {
#ifndef NDEBUG
    if (!m_rules.empty()) {
        const Rule& last = m_rules.back();

        if (rule.stage < last.stage)
            throw RuleOrderException("Rule stage regression");
    }
#endif

    if (HasRule(rule.id))
        throw std::runtime_error("Duplicate rule");

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

                if (!next)
                    continue;

                if (next != expr) {
                    expr = next;
                    changed = true;
                    break;
                }
            }
        }
    }

    return expr;
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

        current = AST::ExprIntern::Intern(n);
    }

    return ApplyOnce(current);
}

Expr* RuleEngine::ApplyUntilStable(Expr* expr) const {
    expr = AST::ExprIntern::Intern(expr);

    while (true) {
        Expr* next = ApplyRecursive(expr);
        next = AST::ExprIntern::Intern(next);

        if (next == expr)
            return expr;

        expr = next;
    }
}

} // namespace BitFlow::Core::Rules