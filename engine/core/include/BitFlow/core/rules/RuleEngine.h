#pragma once

#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {

struct Rule;

class RuleEngine {
  private:
    std::vector<Rule> rules;

  public:
    void AddRule(const Rule& r);
    AST::Expr* ApplyOnce(AST::Expr* expr) const;
    AST::Expr* ApplyRecursive(AST::Expr* expr) const;
    AST::Expr* ApplyUntilStable(AST::Expr* expr) const;
};

} // namespace BitFlow::Core::Rules