#include <BitFlow/engine/core/expression/ExprStore.h>
#include <BitFlow/engine/core/helper/Exception.h>
#include <iostream>
#include <sstream>

namespace BitFlow::Engine::Core::Expression {

#ifdef BitFlow_EXPR_LIFETIME_CHECKS

void ExprDebug::SanityCheck() const {
    if (!m_store) {
        std::cerr << "[DebugExpr] sanity check failed: null store" << std::endl;

        BF_CORE_THROW("DebugExpr sanity check failed: null store");
    }

    if (m_store->m_generation != m_generation) {
        std::ostringstream ss;

        ss << "[DebugExpr] lifetime violation detected\n"
           << "  ExprId     : " << m_id.value() << '\n'
           << "  Generation : " << m_generation << '\n'
           << "  Store Gen  : " << m_store->m_generation << '\n';

        std::cerr << ss.str() << std::endl;

        BF_CORE_THROW(ss.str());
    }
}

#endif

} // namespace BitFlow::Engine::Core::Expression
