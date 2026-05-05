#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Add rules
void RuleEngine::AddRule(const Rule& rule) {
    if (!m_present.insert(rule.key).second) {
        _ASSERT(false && "Duplicate RuleKey in RuleEngine");
        return;
    }

    for (const auto& dep : rule.deps) {
        if (m_present.find(dep) == m_present.end()) {
            _ASSERT(false && "Missing rule dependency");
        }
    }

    m_rules.push_back(rule); // by value!
}
#pragma endregion

void RuleEngine::Merge(const RuleEngine& other) {
    for (const Rule& r : other.m_rules) {
        if (m_present.insert(r.key).second)
            m_rules.push_back(r);
    }
}

#pragma region Execution
ExprId RuleEngine::ApplyOnce(ExprStore* store, ExprId id) const {
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
    auto& e = store->get(id);

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
#pragma endregion

} // namespace BitFlow::Core::Rules