#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <functional>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {
class RuleEngine {
  public:
    using DebugCallback = std::function<void(const AST::Expr* before, const AST::Expr* after, RuleId)>;

  private:
    DebugCallback m_debugCallback;

  protected:
    std::vector<Stage> m_stages;
    std::vector<int> m_stageOrder;
    std::unordered_set<RuleId> m_present;

  public:
    RuleEngine();
    virtual ~RuleEngine() = default;

    virtual void AddRule(const Rule& rule);
    AST::Expr* ApplyOnce(AST::Expr* expr) const;
    AST::Expr* ApplyRecursive(AST::Expr* expr) const;
    AST::Expr* ApplyUntilStable(AST::Expr* expr) const;

    void SetDebugCallback(DebugCallback cb);

  protected:
    virtual void AddRule(Stage& stage, Rule rule);

    bool HasRule(RuleId id) const {
        return m_present.find(id) != m_present.end();
    }

    virtual bool ValidateRule(const Rule& rule) const {
        return true;
    }
};

} // namespace BitFlow::Core::Rules
