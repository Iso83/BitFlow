#pragma region

namespace BitFlow::Engine::Core::Expression {

#ifdef BitFlow_EXPR_LIFETIME_CHECKS

class ExprDebug;
class ExprUnsafeStorage;

template <typename T> class FieldHook {
  private:
    ExprDebug* m_parent{};
    T ExprUnsafeStorage::* m_handler{};

  public:
    FieldHook() = default;

    FieldHook(ExprDebug* parent, T ExprUnsafeStorage::* handler) : m_parent(parent), m_handler(handler) {}

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
