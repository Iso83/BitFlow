#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/ids/ExprId.h>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <unordered_map>

namespace DemoSHA {

class ExprBuilder {
  public:
    explicit ExprBuilder(uint32_t nextId = 1) : m_nextId(nextId) {}

    BitFlow::Core::AST::Expr* Var(const std::string& name) {
        auto* e = NewNode(BitFlow::Core::AST::OpType::Var, {});
        m_names[e->id.value()] = name;
        return e;
    }

    BitFlow::Core::AST::Expr* Const(uint32_t value) {
        return BitFlow::Core::Expression::ConstPool::Get(value);
    }

    BitFlow::Core::AST::Expr* Op(BitFlow::Core::AST::OpType op,
                                 std::initializer_list<BitFlow::Core::AST::Expr*> inputs) {
        return NewNode(op, inputs);
    }

    BitFlow::Core::AST::Expr* Add(std::initializer_list<BitFlow::Core::AST::Expr*> inputs) {
        return Op(BitFlow::Core::AST::OpType::Add, inputs);
    }

    BitFlow::Core::AST::Expr* Xor(std::initializer_list<BitFlow::Core::AST::Expr*> inputs) {
        return Op(BitFlow::Core::AST::OpType::Xor, inputs);
    }

    BitFlow::Core::AST::Expr* And(BitFlow::Core::AST::Expr* a, BitFlow::Core::AST::Expr* b) {
        return Op(BitFlow::Core::AST::OpType::And, {a, b});
    }

    BitFlow::Core::AST::Expr* Shr(BitFlow::Core::AST::Expr* x, uint32_t amount) {
        return Op(BitFlow::Core::AST::OpType::Shr, {x, Const(amount)});
    }

    BitFlow::Core::AST::Expr* RotR(BitFlow::Core::AST::Expr* x, uint32_t amount) {
        return Op(BitFlow::Core::AST::OpType::RotR, {x, Const(amount)});
    }

    BitFlow::Core::AST::Expr* Ch(BitFlow::Core::AST::Expr* x, BitFlow::Core::AST::Expr* y,
                                 BitFlow::Core::AST::Expr* z) {
        return Op(BitFlow::Core::AST::OpType::Ch, {x, y, z});
    }

    BitFlow::Core::AST::Expr* Maj(BitFlow::Core::AST::Expr* x, BitFlow::Core::AST::Expr* y,
                                  BitFlow::Core::AST::Expr* z) {
        return Op(BitFlow::Core::AST::OpType::Maj, {x, y, z});
    }

    BitFlow::Core::AST::Expr* BigSigma0(BitFlow::Core::AST::Expr* x) {
        return Xor({RotR(x, 2), RotR(x, 13), RotR(x, 22)});
    }

    BitFlow::Core::AST::Expr* BigSigma1(BitFlow::Core::AST::Expr* x) {
        return Xor({RotR(x, 6), RotR(x, 11), RotR(x, 25)});
    }

    BitFlow::Core::AST::Expr* SmallSigma0(BitFlow::Core::AST::Expr* x) {
        return Xor({RotR(x, 7), RotR(x, 18), Shr(x, 3)});
    }

    BitFlow::Core::AST::Expr* SmallSigma1(BitFlow::Core::AST::Expr* x) {
        return Xor({RotR(x, 17), RotR(x, 19), Shr(x, 10)});
    }

    const std::unordered_map<uint32_t, std::string>& Names() const {
        return m_names;
    }

  private:
    BitFlow::Core::AST::Expr* NewNode(BitFlow::Core::AST::OpType op,
                                      std::initializer_list<BitFlow::Core::AST::Expr*> inputs) {
        auto* e = new BitFlow::Core::AST::Expr{};
        e->id = BitFlow::Core::Ids::ExprId{m_nextId++};
        e->op = op;
        e->inputs = inputs;
        return e;
    }

    uint32_t m_nextId;
    std::unordered_map<uint32_t, std::string> m_names;
};

} // namespace DemoSHA
