#pragma once

#include <BitFlow/core/ExprIntern.h>
#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>
#include <vector>

namespace BitFlow::Core {

class RuleEngine {
  public:
    void AddRule(const Rule& r) {
        rules.push_back(r);
    }

    // Expr* ApplyOnce(Expr* expr) const {
    //     bool changed = true;

    //    while (changed) {
    //        changed = false;

    //        for (const auto& r : rules) {
    //            if (r.match(*expr)) {
    //                Expr* next = r.rewrite(*expr);

    //                if (next != expr) {
    //                    expr = next;
    //                    changed = true;
    //                    break; // restart rules op nieuwe expr
    //                }
    //            }
    //        }
    //    }

    //    return expr;
    //}

    Expr* ApplyOnce(Expr* expr) const {
        bool changed = true;

        while (changed) {
            changed = false;

            for (const auto& r : rules) {
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

        return ExprIntern::Intern(expr);
    }

    Expr* ApplyRecursive(Expr* expr) const {
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