#pragma once

#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/ExprRef.h>
#include <BitFlow/core/helper/Attributes.h>
#include <stdexcept>
#include <unordered_map>

namespace BitFlow::Core::Expression {

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
        expr.op = op;
        expr.bitWidth = bitWidth;
        expr.inputs = in;

        m_alive[index] = true;

        return ExprRef(this, id);
    }

    [[nodiscard]] ExprRef create(OpType op, std::vector<Ids::ExprId>&& in, uint16_t bitWidth = 64) {
        _ASSERT(bitWidth > 0);

        const auto id = createId();
        const auto index = toIndex(id);

        ensureCapacity(index + 1);

        auto& expr = m_nodes[index];
        expr = Expr{};
        expr.op = op;
        expr.bitWidth = bitWidth;
        expr.inputs = std::move(in);

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

    [[nodiscard]] ExprRef makeFalse(uint16_t bitWidth = 64) {
        return createConstant(0ULL, bitWidth);
    }

    [[nodiscard]] ExprRef makeTrue(uint16_t bitWidth = 64) {
        auto ref = create(OpType::Const, {}, bitWidth);

        auto& expr = get(ref);
        expr.knownMask = Expr::fullMask(bitWidth);
        expr.knownValue = expr.knownMask;

        return ref;
    }

    [[nodiscard]] ExprRef invertConst(Ids::ExprId id) {
        const Expr& e = get(id);

        if (e.op != OpType::Const)
            throw std::runtime_error("ExprStore::invertConst expects Const");

        const uint64_t mask = Expr::fullMask(e.bitWidth);
        const uint64_t value = (~e.knownValue) & mask;

        return createConstant(value, e.bitWidth);
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