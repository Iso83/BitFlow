#pragma once

#include <BitFlow/core/expression/ExprRef.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/helper/Attributes.h>
#include <unordered_map>

namespace BitFlow::Core::Expression {

BF_DEPRECATED("Use ExprRef & ExprStore")
inline ExprOld* CloneExpr(const ExprOld* e) {
    ExprOld* n = new ExprOld{};
    n->op = e->op;
    n->constValue = e->constValue;
    n->inputs = e->inputs;
    return n;
}

BF_DEPRECATED("Use ExprRef & ExprStore")
inline ExprOld* Clone(const ExprOld* expr) {
    std::unordered_map<const ExprOld*, ExprOld*> cloned{};

    const auto cloneNode = [&](const auto& self, const ExprOld* node) -> ExprOld* {
        auto it = cloned.find(node);
        if (it != cloned.end())
            return it->second;

        ExprOld* out = new ExprOld{};
        out->id = node->id;
        out->op = node->op;
        out->constValue = node->constValue;
        cloned.emplace(node, out);

        out->inputs.reserve(node->inputs.size());
        for (const ExprOld* input : node->inputs) {
            out->inputs.push_back(self(self, input));
        }

        return out;
    };

    return cloneNode(cloneNode, expr);
}

BF_DEPRECATED("Use ExprRef & ExprStore")
inline bool StructEqual(const ExprOld* a, const ExprOld* b) {
    std::unordered_map<const ExprOld*, const ExprOld*> seen{};

    const auto eqNode = [&](const auto& self, const ExprOld* lhs, const ExprOld* rhs) -> bool {
        if (lhs == rhs)
            return true;
        if (!lhs || !rhs)
            return lhs == rhs;

        auto seenIt = seen.find(lhs);
        if (seenIt != seen.end())
            return seenIt->second == rhs;

        if (lhs->op != rhs->op || lhs->constValue != rhs->constValue || lhs->inputs.size() != rhs->inputs.size())
            return false;

        seen.emplace(lhs, rhs);
        for (size_t i = 0; i < lhs->inputs.size(); ++i) {
            if (!self(self, lhs->inputs[i], rhs->inputs[i]))
                return false;
        }

        return true;
    };

    return eqNode(eqNode, a, b);
}

BF_DEPRECATED("Use ExprRef & ExprStore")
ExprOld* MakeOpInterned(OpType op, std::vector<ExprOld*> inputs);

BF_DEPRECATED("Use ExprRef & ExprStore")
class ConstPool {
  public:
    static ExprOld* Get(uint32_t value) {
        auto it = pool().find(value);
        if (it != pool().end())
            return it->second;

        ExprOld* e = new ExprOld{};
        e->op = OpType::Const;
        e->constValue = value;
        e->id = Ids::ExprId{NextId()};

        pool()[value] = e;
        return e;
    }

  private:
    static std::unordered_map<uint32_t, ExprOld*>& pool() {
        static std::unordered_map<uint32_t, ExprOld*> p;
        return p;
    }

    static uint32_t NextId() {
        static uint32_t id = 1000000;
        return id++;
    }
};
#pragma endregion

class ExprStore {
  private:
    using ValueType = Ids::ExprId::ValueType;

    ValueType m_nextId{1};
    std::vector<ValueType> m_freeIds{};

    std::vector<Expr> m_nodes{};
    std::vector<bool> m_alive{};

  public:
    ExprStore() = default;
    ~ExprStore() = default;

    [[nodiscard]] ExprRef create(OpType op, std::initializer_list<Ids::ExprId> in, uint16_t bitWidth = 64) {
        _ASSERT(bitWidth > 0);

        const auto id = createId();
        const auto index = toIndex(id);

        ensureCapacity(index + 1);

        auto& expr = m_nodes[index];
        expr = Expr{};
        expr.id = id;
        expr.op = op;
        expr.bitWidth = bitWidth;
        expr.inputs = in;

        m_alive[index] = true;

        return ExprRef(this, id);
    }

    [[nodiscard]] ExprRef createConstant(uint64_t value, uint16_t bitWidth = 64) {
        auto ref = create(OpType::Const, {}, bitWidth);

        auto& expr = get(ref);
        expr.knownMask = Expr::fullMask(bitWidth);
        expr.knownValue = value;

        return ref;
    }

    [[nodiscard]] ExprRef createVariable(uint16_t bitWidth = 64) {
        return create(OpType::Var, {}, bitWidth);
    }

    [[nodiscard]] bool remove(ExprRef ref) {
        if (!contains(ref)) {
            return false;
        }

        const auto index = toIndex(ref.id);

        m_alive[index] = false;
        m_freeIds.push_back(ref.id.value());

        return true;
    }

    [[nodiscard]] bool contains(ExprRef ref) const {
        if (ref.store != this || ref.id.value() == 0) {
            return false;
        }

        const auto index = toIndex(ref.id);

        if (index >= m_alive.size()) {
            return false;
        }

        return m_alive[index];
    }

    [[nodiscard]] Expr& get(ExprRef ref) {
        return m_nodes[toIndex(ref.id)];
    }

    [[nodiscard]] Expr& get(Ids::ExprId id) {
        return m_nodes[toIndex(id)];
    }

    [[nodiscard]] const Expr& get(ExprRef ref) const {
        return m_nodes[toIndex(ref.id)];
    }

    [[nodiscard]] const Expr& get(const Ids::ExprId id) const {
        return m_nodes[toIndex(id)];
    }

    [[nodiscard]] Expr& operator[](ExprRef ref) {
        return get(ref);
    }

    [[nodiscard]] const Expr& operator[](ExprRef ref) const {
        return get(ref);
    }

  private:
    [[nodiscard]] Ids::ExprId createId() {
        ValueType value{};

        if (!m_freeIds.empty()) {
            value = m_freeIds.back();
            m_freeIds.pop_back();
        } else {
            value = m_nextId++;
        }

        return Ids::ExprId{value};
    }

    [[nodiscard]] static size_t toIndex(Ids::ExprId id) {
        return static_cast<size_t>(id.value());
    }

    void ensureCapacity(size_t size) {
        if (m_nodes.size() < size) {
            m_nodes.resize(size);
            m_alive.resize(size, false);
        }
    }
};

} // namespace BitFlow::Core::Expression