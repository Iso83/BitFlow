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
    if (!m_stageOrder.empty()) {
        if (rule.stage < m_stageOrder.back())
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

    auto stageIt = std::find(m_stageOrder.begin(), m_stageOrder.end(), rule.stage);
    if (stageIt == m_stageOrder.end()) {
        Stage stage{};
        stage.Name = "Stage " + std::to_string(rule.stage);
        m_stages.push_back(std::move(stage));
        m_stageOrder.push_back(rule.stage);
        stageIt = std::prev(m_stageOrder.end());
    }

    const auto stageIndex = static_cast<size_t>(std::distance(m_stageOrder.begin(), stageIt));
    AddRule(m_stages[stageIndex], rule);
    m_present.insert(rule.id);
}

void RuleEngine::AddRule(Stage& stage, Rule rule) {
    if (stage.ruleIds.find(rule.Id) != stage.ruleIds.end())
        throw std::runtime_error("Duplicate rule in stage");

    stage.rules.push_back(std::move(rule));
    stage.ruleIds.insert(stage.rules.back().Id);
}

Expr* RuleEngine::ApplyOnce(Expr* expr) const {
    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& stage : m_stages) {
            for (const auto& r : stage.rules) {
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

            if (changed)
                break;
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
