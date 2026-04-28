#include "RuleOrderException.h"
#include "expression/ExprIntern.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <algorithm>

namespace BitFlow::Core::Rules {

using Expr = Expression::ExprOld;

RuleEngine::RuleEngine() {
    Expression::ExprIntern::Reset();
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
    if (!stage.ruleIds.insert(rule.Id).second)
        throw std::runtime_error("Duplicate rule in stage");

    bool hasExpandOnly = false;
    bool hasFactorOnly = false;

    for (const auto& stageRule : stage.rules) {
        const bool isExpand = stageRule.IsExpanding();
        const bool isFactor = stageRule.IsFactorizing();

        if (isExpand && !isFactor)
            hasExpandOnly = true;

        if (isFactor && !isExpand)
            hasFactorOnly = true;
    }

    const bool ruleIsExpand = rule.IsExpanding();
    const bool ruleIsFactor = rule.IsFactorizing();

    if (ruleIsExpand && !ruleIsFactor)
        hasExpandOnly = true;

    if (ruleIsFactor && !ruleIsExpand)
        hasFactorOnly = true;

    if (hasExpandOnly && hasFactorOnly) {
        stage.ruleIds.erase(rule.Id);
        throw std::runtime_error("Invalid stage: expand + factorize");
    }

    stage.rules.push_back(std::move(rule));
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

                    next = Expression::ExprIntern::Intern(next);

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

        current = Expression::ExprIntern::Intern(n);
    } else
        current = Expression::ExprIntern::Intern(current);

    return ApplyOnce(current);
}

RewriteResult RuleEngine::RewriteToFixedPoint(Expr* expr) const {
    expr = Expression::ExprIntern::Intern(expr);
    std::vector<Expr*> history{};
    history.push_back(Expression::Clone(expr));
    constexpr size_t maxIterations = 64;

    size_t iterations = 0;
    while (iterations < maxIterations) {
        Expr* next = ApplyRecursive(expr);
        next = Expression::ExprIntern::Intern(next);

        if (Expression::StructEqual(next, expr))
            return RewriteResult{next, true, false, static_cast<int>(iterations)};

        for (const Expr* prev : history) {
            if (Expression::StructEqual(prev, next))
                return RewriteResult{next, false, true, static_cast<int>(iterations + 1)};
        }

        history.push_back(Expression::Clone(next));
        expr = next;
        ++iterations;
    }

    return RewriteResult{expr, false, false, static_cast<int>(iterations)};
}

void RuleEngine::SetDebugCallback(DebugCallback cb) {
    m_debugCallback = std::move(cb);
}

} // namespace BitFlow::Core::Rules
