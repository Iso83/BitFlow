#pragma once

#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace BitFlow::Core::Expression {

#ifdef BF_EXPR_LIFETIME_CHECKS

void Expr::SanityCheck() const {
    if (!m_store) {
        std::cerr << "[DebugExpr] sanity check failed: null store" << std::endl;

        throw std::runtime_error("DebugExpr sanity check failed: null store");
    }

    if (m_store->m_generation != m_generation) {
        std::ostringstream ss;

        ss << "[DebugExpr] lifetime violation detected\n"
           << "  ExprId     : " << m_id.value() << '\n'
           << "  Generation : " << m_generation << '\n'
           << "  Store Gen  : " << m_store->m_generation << '\n';

        std::cerr << ss.str() << std::endl;

        throw std::runtime_error(ss.str());
    }
}

#endif

} // namespace BitFlow::Core::Expression