#pragma once

#include <BitFlow/core/rules/DependencyValidationResult.h>
#include <BitFlow/core/rules/Rule.h>
#include <functional>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Expression {
class ExprStore;
}

namespace BitFlow::Core::Rules {

class RuleEngine {
  public:
    int maxIterations = 64;

    struct DebugCallBack_Ctx {
        RuleKey key;
        Expression::ExprStore* store = nullptr;
        std::function<void(Ids::ExprId)> beginCallback;
        std::function<void(Ids::ExprId)> endCallback;
    };
    using DebugCallback = std::function<void(DebugCallBack_Ctx& ctx)>;

  private:
    DebugCallback m_debugCallback;

    std::vector<Rule> m_rules; // ordered
    std::unordered_set<RuleKey> m_present;

    mutable bool m_validated{false};

  public:
    RuleEngine() = default;
    ~RuleEngine() = default;

    [[nodiscard]] inline bool Contains(const RuleKey& key) const {
        return m_present.contains(key);
    }

    void AddRule(const Rule& rule);

    bool RemoveRule(const RuleKey& key);

    void Merge(const RuleEngine& other) {
        for (const auto& rule : other.m_rules)
            AddRule(rule);
    }

    const std::vector<Rule>& Rules() const {
        return m_rules;
    }

    [[nodiscard]] Ids::ExprId ApplyOnce(Expression::ExprStore* store, Ids::ExprId id,
                                        const Expression::ExprNameMap* names = nullptr) const;
    [[nodiscard]] Ids::ExprId ApplyRecursive(Expression::ExprStore* store, Ids::ExprId id,
                                             const Expression::ExprNameMap* names = nullptr) const;
    [[nodiscard]] Ids::ExprId Rewrite(Expression::ExprStore* store, Ids::ExprId root,
                                      const Expression::ExprNameMap* names = nullptr) const;

    void SetDebugCallback(DebugCallback cb) {
        m_debugCallback = std::move(cb);
    }

    DependencyValidationResult AnalyzeDependencies(const Rule& testingRule) const;
    std::unordered_set<RuleKey> CollectRequiredRules(const RuleKey& key) const;

  private:
    void ValidateDependencies() const;
};

} // namespace BitFlow::Core::Rules