#include <BitFlow/engine/core/expression/Expr.h>
#include <BitFlow/engine/core/helper/CheckedExprInputs.h>

namespace BitFlow::Engine::Core::Expression {

#ifdef BitFlow_EXPR_LIFETIME_CHECKS

CheckedExprInputs::CheckedExprInputs(ExprDebug* parent) : m_parent(parent) {}

bool CheckedExprInputs::empty() const {
    Validate();
    return m_parent->m_expr->inputs.empty();
}

size_t CheckedExprInputs::size() const {
    Validate();
    return m_parent->m_expr->inputs.size();
}

Ids::ExprId& CheckedExprInputs::operator[](size_t i) {
    Validate();
    return m_parent->m_expr->inputs[i];
}

const Ids::ExprId& CheckedExprInputs::operator[](size_t i) const {
    Validate();
    return m_parent->m_expr->inputs[i];
}

Ids::ExprId& CheckedExprInputs::front() {
    Validate();
    return m_parent->m_expr->inputs.front();
}

const Ids::ExprId& CheckedExprInputs::front() const {
    Validate();
    return m_parent->m_expr->inputs.front();
}

Ids::ExprId& CheckedExprInputs::back() {
    Validate();
    return m_parent->m_expr->inputs.back();
}

const Ids::ExprId& CheckedExprInputs::back() const {
    Validate();
    return m_parent->m_expr->inputs.back();
}

CheckedExprInputs::iterator CheckedExprInputs::begin() {
    Validate();
    return m_parent->m_expr->inputs.begin();
}

CheckedExprInputs::iterator CheckedExprInputs::end() {
    Validate();
    return m_parent->m_expr->inputs.end();
}

CheckedExprInputs::const_iterator CheckedExprInputs::begin() const {
    Validate();
    return m_parent->m_expr->inputs.begin();
}

CheckedExprInputs::const_iterator CheckedExprInputs::end() const {
    Validate();
    return m_parent->m_expr->inputs.end();
}

CheckedExprInputs::const_iterator CheckedExprInputs::cbegin() const {
    Validate();
    return m_parent->m_expr->inputs.cbegin();
}

CheckedExprInputs::const_iterator CheckedExprInputs::cend() const {
    Validate();
    return m_parent->m_expr->inputs.cend();
}

bool CheckedExprInputs::contains(Ids::ExprId id) const {
    Validate();

    return std::find(m_parent->m_expr->inputs.begin(), m_parent->m_expr->inputs.end(), id) !=
           m_parent->m_expr->inputs.end();
}

CheckedExprInputs::operator const Container&() const {
    Validate();
    return m_parent->m_expr->inputs;
}

void CheckedExprInputs::Validate() const {
    m_parent->SanityCheck();
}

bool CheckedExprInputs::operator==(const CheckedExprInputs& other) const {
    Validate();
    other.Validate();

    return m_parent->m_expr->inputs == other.m_parent->m_expr->inputs;
}

bool CheckedExprInputs::operator==(const Container& other) const {
    Validate();
    return m_parent->m_expr->inputs == other;
}

bool operator==(const CheckedExprInputs::Container& lhs, const CheckedExprInputs& rhs) {
    rhs.Validate();
    return rhs == lhs;
}

bool operator!=(const CheckedExprInputs::Container& lhs, const CheckedExprInputs& rhs) {
    return !(lhs == rhs);
}
#endif

} // namespace BitFlow::Engine::Core::Expression
