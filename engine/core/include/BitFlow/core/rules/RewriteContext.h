// RewriteContext.h

#pragma once

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/helper/Attributes.h>

namespace BitFlow::Core::Rules {

class RuleEngine;

class RewriteContext final {
    friend class RuleEngine;

  private:
    Expression::ExprStore* m_store;

    explicit RewriteContext(Expression::ExprStore* store) : m_store(store) {}

  public:
    bool changed = false;

    inline void replace(Ids::ExprId oldId, Ids::ExprId newId) {
        m_store->replace(oldId, newId);
        changed = true;
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