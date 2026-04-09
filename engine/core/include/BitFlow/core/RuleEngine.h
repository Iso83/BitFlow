#pragma once

#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>
#include <vector>

namespace BitFlow::Core {

class RuleEngine {
  public:
    void AddRule(const Rule& r) {
        rules.push_back(r);
    }

    Expr* ApplyOnce(Expr* expr) const {
        for (const auto& r : rules) {
            if (r.match(*expr)) {
                return r.rewrite(*expr);
            }
        }
        return expr;
    }

    Expr* ApplyRecursive(Expr* expr) const {
        for (auto& input : expr->inputs) {
            input = ApplyRecursive(input);
        }

        return ApplyOnce(expr);
    }

    Expr* ApplyUntilStable(Expr* expr) const {
        while (true) {
            Expr* next = ApplyRecursive(expr);

            if (next == expr)
                return expr;

            expr = next;
        }
    }

  private:
    std::vector<Rule> rules;
};

} // namespace BitFlow::Core