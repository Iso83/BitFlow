// CheckedExprInputs.h
#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <algorithm>
#include <vector>

namespace BitFlow::Core::Expression {

#ifdef BF_EXPR_LIFETIME_CHECKS

class Expr;
class _Expr_INTERNALONLY;

class CheckedExprInputs {
  public:
    using Container = std::vector<Ids::ExprId>;

    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

  private:
    Expr* m_parent{};

  public:
    CheckedExprInputs() = default;
    explicit CheckedExprInputs(Expr* parent);

  public:
    bool empty() const;
    size_t size() const;

    Ids::ExprId& operator[](size_t i);
    const Ids::ExprId& operator[](size_t i) const;

    Ids::ExprId& front();
    const Ids::ExprId& front() const;

    Ids::ExprId& back();
    const Ids::ExprId& back() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    bool contains(Ids::ExprId id) const;

    operator const Container&() const;

    bool operator==(const CheckedExprInputs& other) const;

    bool operator!=(const CheckedExprInputs& other) const {
        return !(*this == other);
    }

    bool operator==(const Container& other) const;

    bool operator!=(const Container& other) const {
        return !(*this == other);
    }

    friend bool operator==(const Container& lhs, const CheckedExprInputs& rhs);

    friend bool operator!=(const Container& lhs, const CheckedExprInputs& rhs);

    CheckedExprInputs& operator=(const CheckedExprInputs&) = delete;

  private:
    void Validate() const;
};

#endif

} // namespace BitFlow::Core::Expression