#pragma once

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
    using DebugCallback = std::function<void(Ids::ExprId before, Ids::ExprId after, RuleKey key)>;

  private:
    DebugCallback m_debugCallback;

    std::vector<Rule> m_rules; // ordered
    std::unordered_set<RuleKey> m_present;

  public:
    RuleEngine() = default;
    ~RuleEngine() = default;

    void AddRule(const Rule& rule);

    void Merge(const RuleEngine& other);

    Ids::ExprId ApplyOnce(Expression::ExprStore* store, Ids::ExprId id) const;
    Ids::ExprId ApplyRecursive(Expression::ExprStore* store, Ids::ExprId id) const;
    Ids::ExprId Rewrite(Expression::ExprStore* store, Ids::ExprId root) const;

    void SetDebugCallback(DebugCallback cb) {
        m_debugCallback = std::move(cb);
    }
};

} // namespace BitFlow::Core::Rules