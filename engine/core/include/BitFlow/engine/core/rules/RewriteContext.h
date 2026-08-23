#pragma once

#include <BitFlow/engine/core/expression/ExprStore.h>

namespace BitFlow::Engine::Core::Rules {

class RuleEngine;

class RewriteContext final {
    friend class RuleEngine;

  private:
    Expression::ExprStore* m_store;
    bool changed = false;

    explicit RewriteContext(Expression::ExprStore* store) : m_store(store) {}

  public:
    inline Ids::ExprId replace(Ids::ExprId oldId, Ids::ExprId newId) {
        m_store->replace(oldId, newId);
        changed = true;
        return newId;
    }

    inline void release(Ids::ExprId oldId) {
        m_store->release(oldId);
        changed = true;
    }

    [[nodiscard]] operator Expression::ExprStore*() const {
        return m_store;
    }
};

} // namespace BitFlow::Core::Rules
