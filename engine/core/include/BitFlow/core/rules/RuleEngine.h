#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <functional>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {

struct RewriteResult {
    AST::Expr* result;
    bool stable;
    bool oscillationDetected;
    int iterations;

    bool cycleDetected() const {
        return oscillationDetected;
    }

    bool StableWithoutCycle() const {
        return stable && !cycleDetected();
    }

    bool IterationsWithin(int limit) const {
        return iterations <= limit;
    }
};

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
    RewriteResult RunWithInfo(AST::Expr* expr) const;

    void SetDebugCallback(DebugCallback cb);

    const std::vector<Stage>& Stages() const {
        return m_stages;
    }

    const std::vector<int>& StageOrder() const {
        return m_stageOrder;
    }

  protected:
    virtual void AddRule(Stage& stage, Rule rule);

    bool HasRule(RuleId id) const {
        return m_present.find(id) != m_present.end();
    }

    virtual bool ValidateRule(const Rule& rule) const {
        return rule.Name != nullptr && rule.Name[0] != '\0';
    }
};

} // namespace BitFlow::Core::Rules
