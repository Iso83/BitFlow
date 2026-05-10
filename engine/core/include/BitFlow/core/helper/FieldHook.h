#pragma region

namespace BitFlow::Core::Expression {

#ifdef BF_EXPR_LIFETIME_CHECKS

class Expr;
class _Expr_INTERNALONLY;

template <typename T> class FieldHook {
  private:
    Expr* m_parent{};
    T _Expr_INTERNALONLY::* m_handler{};

  public:
    FieldHook() = default;

    FieldHook(Expr* parent, T _Expr_INTERNALONLY::* handler) : m_parent(parent), m_handler(handler) {}

    operator T&() {
        Validate();
        return (m_parent->m_expr)->*m_handler;
    }

    operator const T&() const {
        Validate();
        return (m_parent->m_expr)->*m_handler;
    }

    T* operator->() {
        Validate();
        return &((m_parent->m_expr)->*m_handler);
    }

    const T* operator->() const {
        Validate();
        return &((m_parent->m_expr)->*m_handler);
    }

    FieldHook& operator=(const T& v) {
        Validate();
        (m_parent->m_expr)->*m_handler = v;
        return *this;
    }

    FieldHook& operator=(const FieldHook&) = delete;

  private:
    void Validate() const {
        m_parent->SanityCheck();
    }
};

#endif

} // namespace BitFlow::Core::Expression