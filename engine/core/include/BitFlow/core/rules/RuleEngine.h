#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {
class RuleEngine {
  protected:
    std::vector<Rule> m_rules;
    std::unordered_set<RuleId> m_present;

  public:
    virtual ~RuleEngine() = default;

    virtual void AddRule(const Rule& rule);
    AST::Expr* ApplyOnce(AST::Expr* expr) const;
    AST::Expr* ApplyRecursive(AST::Expr* expr) const;
    AST::Expr* ApplyUntilStable(AST::Expr* expr) const;

  protected:
    bool HasRule(RuleId id) const {
        return m_present.find(id) != m_present.end();
    }

    virtual bool ValidateRule(const Rule& rule) const {
        return true;
    }
};

} // namespace BitFlow::Core::Rules