#include "RuleOrderException.h"
#include "ast/ExprIntern.h"
#include "expression/ExprClone.h"

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

    for (uint32_t dep : rule.Dependencies) {
        if (!HasRule(static_cast<RuleId>(dep))) {
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
                Expr* before = expr;
                Expr* mutableInput = Expression::CloneExpr(expr);
                Expr* next = r.rewrite(*mutableInput);

                if (!next)
                    continue;

                next = AST::ExprIntern::Intern(next);

                if (next != expr) {
                    if (m_debugCallback) {
                        m_debugCallback(before, next, r.id);
                    }

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
        n->constValue = expr->constValue;
        n->inputs = std::move(newInputs);

        current = AST::ExprIntern::Intern(n);
    } else
        current = AST::ExprIntern::Intern(current);

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

void RuleEngine::SetDebugCallback(DebugCallback cb) {
    m_debugCallback = std::move(cb);
}

} // namespace BitFlow::Core::Rules
