#include "expression/ExprUtils.h"
#include "rules/RuleDiagnostics.h"

#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

void RuleEngine::AddRule(const Rule& rule) {
    if (m_present.contains(rule.key))
        return;

    m_present.insert(rule.key);
    m_rules.push_back(rule);

    m_validated = false;
}

ExprId RuleEngine::ApplyOnce(ExprStore* store, ExprId id) const {
    if (!m_validated)
        ValidateDependencies();

    for (const Rule& r : m_rules) {
        if (!r.match(store, id))
            continue;

        const ExprId before = id;
        const ExprId after = r.rewrite(store, id);

        if (after != before) {
            if (m_debugCallback)
                m_debugCallback(before, after, r.key);

            return after;
        }
    }

    return id;
}

ExprId RuleEngine::ApplyRecursive(ExprStore* store, ExprId id) const {
    Expr& e = store->get(id);

    bool changed = false;

    for (auto& child : e.inputs) {
        ExprId newChild = ApplyRecursive(store, child);
        if (newChild != child) {
            child = newChild;
            changed = true;
        }
    }

    return ApplyOnce(store, id);
}

ExprId RuleEngine::Rewrite(ExprStore* store, ExprId root) const {
    ExprId current = root;

    constexpr int maxIterations = 64;

    for (int i = 0; i < maxIterations; ++i) {
        ExprId next = ApplyRecursive(store, current);

        if (next == current)
            return current;

        current = next;
    }

    _ASSERT(false && "Rewrite did not converge");
    return current;
}

void CollectDependenciesRecursive(const std::vector<Rule>& rules, const std::unordered_map<RuleKey, size_t>& indices,
                                  const Rule& rule, std::unordered_set<RuleKey>& out) {

    for (const auto& dep : rule.deps) {

        if (!out.insert(dep).second)
            continue;

        const auto it = indices.find(dep);

        if (it == indices.end())
            continue;

        const Rule& depRule = rules[it->second];

        CollectDependenciesRecursive(rules, indices, depRule, out);
    }
}

DependencyValidationResult RuleEngine::ValidateMinimalDependencies(const Rule& testingRule) const {
    DependencyValidationResult result(testingRule.key);

    std::unordered_map<RuleKey, size_t> indices;

    indices.reserve(m_rules.size());

    for (size_t i = 0; i < m_rules.size(); ++i)
        indices.emplace(m_rules[i].key, i);

    std::unordered_set<RuleKey> required;

    CollectDependenciesRecursive(m_rules, indices, testingRule, required);

    // --- missing ---
    for (const auto& key : required) {
        if (!m_present.contains(key)) {
            result.valid = false;
            result.missing.push_back(key);
        }
    }

    // --- extra ---
    for (const auto& rule : m_rules) {
        if (rule.key == testingRule.key)
            continue;

        if (!required.contains(rule.key)) {
            result.valid = false;
            result.extra.push_back(rule.key);
        }
    }

    return result;
}

void RuleEngine::ValidateDependencies() const {
    std::unordered_map<RuleKey, size_t> indices;

    indices.reserve(m_rules.size());

    for (size_t i = 0; i < m_rules.size(); ++i)
        indices.emplace(m_rules[i].key, i);

    for (size_t i = 0; i < m_rules.size(); ++i) {
        const auto& rule = m_rules[i];

        for (const auto& dep : rule.deps) {
            const auto it = indices.find(dep);

            if (it == indices.end())
                BF_RULE_ERROR("Missing dependency: " + std::string(dep.value) + " required by " +
                              std::string(rule.key.value));

            if (it->second >= i)
                BF_RULE_ERROR("Dependency order invalid: " + std::string(dep.value) + " must execute before " +
                              std::string(rule.key.value));
        }
    }

    m_validated = true;
}

} // namespace BitFlow::Core::Rules