#pragma once

#include <Core_Expr.h>
#include <cstdint>
#include <initializer_list>

namespace BitFlow::Core::Testing::SHA {

class Builder {
  public:
    explicit Builder(uint32_t nextId = 1000) : m_nextId(nextId) {}

    Expression::Expr* Var() {
        return MakeVar(NextId());
    }

    Expression::Expr* Const(uint32_t value) {
        return MakeConst(NextId(), value);
    }

    Expression::Expr* Not(Expression::Expr* x) {
        return MakeOp(NextId(), Expression::OpType::Not, {x});
    }

    Expression::Expr* And(Expression::Expr* a, Expression::Expr* b) {
        return MakeOp(NextId(), Expression::OpType::And, {a, b});
    }

    Expression::Expr* Xor(std::initializer_list<Expression::Expr*> terms) {
        return MakeOp(NextId(), Expression::OpType::Xor, terms);
    }

    Expression::Expr* Add(std::initializer_list<Expression::Expr*> terms) {
        return MakeOp(NextId(), Expression::OpType::Add, terms);
    }

    Expression::Expr* RotR(Expression::Expr* x, uint32_t amount) {
        return MakeOp(NextId(), Expression::OpType::RotR, {x, Const(amount)});
    }

    Expression::Expr* RotL(Expression::Expr* x, uint32_t amount) {
        return MakeOp(NextId(), Expression::OpType::RotL, {x, Const(amount)});
    }

    Expression::Expr* Ch(Expression::Expr* x, Expression::Expr* y, Expression::Expr* z) {
        return MakeOp(NextId(), Expression::OpType::Ch, {x, y, z});
    }

    Expression::Expr* Maj(Expression::Expr* x, Expression::Expr* y, Expression::Expr* z) {
        return MakeOp(NextId(), Expression::OpType::Maj, {x, y, z});
    }

    // SHA-256 upper-case sigma helpers.
    Expression::Expr* BigSigma0(Expression::Expr* x) {
        return Xor({RotR(x, 2), RotR(x, 13), RotR(x, 22)});
    }

    Expression::Expr* BigSigma1(Expression::Expr* x) {
        return Xor({RotR(x, 6), RotR(x, 11), RotR(x, 25)});
    }

    // SHA-256 lower-case sigma helpers.
    Expression::Expr* SmallSigma0(Expression::Expr* x) {
        return Xor({RotR(x, 7), RotR(x, 18), MakeOp(NextId(), Expression::OpType::Shr, {x, Const(3)})});
    }

    Expression::Expr* SmallSigma1(Expression::Expr* x) {
        return Xor({RotR(x, 17), RotR(x, 19), MakeOp(NextId(), Expression::OpType::Shr, {x, Const(10)})});
    }

    Expression::Expr* RoundT1ChoiceCore(Expression::Expr* e, Expression::Expr* f, Expression::Expr* g) {
        return Ch(e, f, g);
    }

    Expression::Expr* RoundT1SigmaChoiceCore(Expression::Expr* e, Expression::Expr* f, Expression::Expr* g) {
        return Add({BigSigma1(e), RoundT1ChoiceCore(e, f, g)});
    }

    Expression::Expr* RoundT1PartCore(Expression::Expr* h, Expression::Expr* e, Expression::Expr* f,
                                      Expression::Expr* g) {
        return Add({h, RoundT1SigmaChoiceCore(e, f, g)});
    }

    Expression::Expr* RoundT2PartCore(Expression::Expr* a, Expression::Expr* b, Expression::Expr* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

    // Typical SHA-256 round fragments.
    Expression::Expr* RoundT1(Expression::Expr* h, Expression::Expr* e, Expression::Expr* f, Expression::Expr* g,
                              Expression::Expr* k, Expression::Expr* w) {
        return Add({h, BigSigma1(e), Ch(e, f, g), k, w});
    }

    Expression::Expr* RoundT2(Expression::Expr* a, Expression::Expr* b, Expression::Expr* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

  private:
    uint32_t NextId() {
        return m_nextId++;
    }

    uint32_t m_nextId = 0;
};

} // namespace BitFlow::Core::Testing::SHA
