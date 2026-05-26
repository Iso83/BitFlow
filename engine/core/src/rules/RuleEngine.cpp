#include "expression/ExprUtils.h"
#include "rules/RuleDiagnostics.h"

#include <BitFlow/core/rules/RewriteContext.h>
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

    RewriteContext ctx(store);

    for (const Rule& r : m_rules) {
        if (!r.match(store, id))
            continue;

        const ExprId before = id;
        const ExprId after = r.rewrite(ctx, id);

        if (after != before) {
            if (m_debugCallback)
                m_debugCallback(before, after, r.key);

            return after;
        }
    }

    return id;
}

ExprId RuleEngine::ApplyRecursive(ExprStore* store, ExprId id) const {
    const Expr& e = (*store)[id];

    const OpType op = e.op;
    const Types::BitWidth bitWidth = e.bitWidth;
    const std::vector<ExprId> inputs = e.inputs;

    std::vector<ExprId> newInputs;
    newInputs.reserve(inputs.size());

    bool changed = false;

    for (ExprId child : inputs) {
        ExprId newChild = ApplyRecursive(store, child);

        if (newChild != child)
            changed = true;

        newInputs.push_back(newChild);
    }

    ExprId current = id;

    if (changed)
        current = store->create(op, std::move(newInputs), bitWidth).id;

    const ExprId rewritten = ApplyOnce(store, current);

    if (rewritten != current)
        return ApplyRecursive(store, rewritten);

    return current;
}

ExprId RuleEngine::Rewrite(ExprStore* store, ExprId root) const {
    ExprId current = root;

    for (int i = 0; i < maxIterations; ++i) {
        ExprId next = ApplyRecursive(store, current);

        if (next == current)
            return current;

        current = next;
    }

    BF_RULE_ERROR("Rewrite did not converge after " + std::to_string(maxIterations) + " iterations");
}

static void CollectDependenciesRecursive(const std::vector<Rule>& rules,
                                         const std::unordered_map<RuleKey, size_t>& indices, const Rule& rule,
                                         std::unordered_set<RuleKey>& out, std::unordered_set<RuleKey>& visited) {

    // prevent cycles / duplicate walks
    if (!visited.insert(rule.key).second)
        return;

    for (const auto& dep : rule.deps) {

        out.insert(dep);

        const auto it = indices.find(dep);

        if (it == indices.end())
            continue;

        const Rule& depRule = rules[it->second];

        CollectDependenciesRecursive(rules, indices, depRule, out, visited);
    }
}

DependencyValidationResult RuleEngine::AnalyzeDependencies(const Rule& testingRule) const {
    DependencyValidationResult result(testingRule.key);
    result.valid = true;

    std::unordered_map<RuleKey, size_t> indices;

    indices.reserve(m_rules.size());

    for (size_t i = 0; i < m_rules.size(); ++i)
        indices.emplace(m_rules[i].key, i);

    // =====================================================
    // Collect full recursive dependency graph
    // =====================================================

    std::unordered_set<RuleKey> required;
    std::unordered_set<RuleKey> visited;

    CollectDependenciesRecursive(m_rules, indices, testingRule, required, visited);

    // =====================================================
    // Missing
    // =====================================================

    for (const auto& key : required) {

        if (!m_present.contains(key)) {
            result.valid = false;
            result.missing.push_back(key);
        }
    }

    // =====================================================
    // Redundant direct dependencies
    // =====================================================

    for (const auto& depA : testingRule.deps) {

        for (const auto& depB : testingRule.deps) {

            if (depA == depB)
                continue;

            const auto it = indices.find(depB);

            if (it == indices.end())
                continue;

            const Rule& depRule = m_rules[it->second];

            std::unordered_set<RuleKey> nested;
            std::unordered_set<RuleKey> nestedVisited;

            CollectDependenciesRecursive(m_rules, indices, depRule, nested, nestedVisited);

            if (nested.contains(depA)) {

#ifdef _DEBUG
                std::cerr << "Redundant dependency detected\n"
                          << " Rule      : " << testingRule.key.value << "\n"
                          << " Dependency: " << depA.value << "\n"
                          << " Via       : " << depB.value << "\n";
#endif

                if (std::find(result.redundant.begin(), result.redundant.end(), depA) == result.redundant.end()) {

                    result.valid = false;
                    result.redundant.push_back(depA);
                }
            }
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
