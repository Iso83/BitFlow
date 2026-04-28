#pragma once

#include <Core_Expr.h>
#include <cstdint>
#include <initializer_list>

namespace BitFlow::Core::Testing::SHA {

class Builder {
  public:
    explicit Builder(uint32_t nextId = 1000) : m_nextId(nextId) {}

    Expression::ExprOld* Var() {
        return MakeVar(NextId());
    }

    Expression::ExprOld* Const(uint32_t value) {
        return MakeConst(NextId(), value);
    }

    Expression::ExprOld* Not(Expression::ExprOld* x) {
        return MakeOp(NextId(), Expression::OpType::Not, {x});
    }

    Expression::ExprOld* And(Expression::ExprOld* a, Expression::ExprOld* b) {
        return MakeOp(NextId(), Expression::OpType::And, {a, b});
    }

    Expression::ExprOld* Xor(std::initializer_list<Expression::ExprOld*> terms) {
        return MakeOp(NextId(), Expression::OpType::Xor, terms);
    }

    Expression::ExprOld* Add(std::initializer_list<Expression::ExprOld*> terms) {
        return MakeOp(NextId(), Expression::OpType::Add, terms);
    }

    Expression::ExprOld* RotR(Expression::ExprOld* x, uint32_t amount) {
        return MakeOp(NextId(), Expression::OpType::RotR, {x, Const(amount)});
    }

    Expression::ExprOld* RotL(Expression::ExprOld* x, uint32_t amount) {
        return MakeOp(NextId(), Expression::OpType::RotL, {x, Const(amount)});
    }

    Expression::ExprOld* Ch(Expression::ExprOld* x, Expression::ExprOld* y, Expression::ExprOld* z) {
        return MakeOp(NextId(), Expression::OpType::Ch, {x, y, z});
    }

    Expression::ExprOld* Maj(Expression::ExprOld* x, Expression::ExprOld* y, Expression::ExprOld* z) {
        return MakeOp(NextId(), Expression::OpType::Maj, {x, y, z});
    }

    // SHA-256 upper-case sigma helpers.
    Expression::ExprOld* BigSigma0(Expression::ExprOld* x) {
        return Xor({RotR(x, 2), RotR(x, 13), RotR(x, 22)});
    }

    Expression::ExprOld* BigSigma1(Expression::ExprOld* x) {
        return Xor({RotR(x, 6), RotR(x, 11), RotR(x, 25)});
    }

    // SHA-256 lower-case sigma helpers.
    Expression::ExprOld* SmallSigma0(Expression::ExprOld* x) {
        return Xor({RotR(x, 7), RotR(x, 18), MakeOp(NextId(), Expression::OpType::Shr, {x, Const(3)})});
    }

    Expression::ExprOld* SmallSigma1(Expression::ExprOld* x) {
        return Xor({RotR(x, 17), RotR(x, 19), MakeOp(NextId(), Expression::OpType::Shr, {x, Const(10)})});
    }

    Expression::ExprOld* RoundT1ChoiceCore(Expression::ExprOld* e, Expression::ExprOld* f, Expression::ExprOld* g) {
        return Ch(e, f, g);
    }

    Expression::ExprOld* RoundT1SigmaChoiceCore(Expression::ExprOld* e, Expression::ExprOld* f,
                                                Expression::ExprOld* g) {
        return Add({BigSigma1(e), RoundT1ChoiceCore(e, f, g)});
    }

    Expression::ExprOld* RoundT1PartCore(Expression::ExprOld* h, Expression::ExprOld* e, Expression::ExprOld* f,
                                         Expression::ExprOld* g) {
        return Add({h, RoundT1SigmaChoiceCore(e, f, g)});
    }

    Expression::ExprOld* RoundT2PartCore(Expression::ExprOld* a, Expression::ExprOld* b, Expression::ExprOld* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

    // Typical SHA-256 round fragments.
    Expression::ExprOld* RoundT1(Expression::ExprOld* h, Expression::ExprOld* e, Expression::ExprOld* f,
                                 Expression::ExprOld* g, Expression::ExprOld* k, Expression::ExprOld* w) {
        return Add({h, BigSigma1(e), Ch(e, f, g), k, w});
    }

    Expression::ExprOld* RoundT2(Expression::ExprOld* a, Expression::ExprOld* b, Expression::ExprOld* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

  private:
    uint32_t NextId() {
        return m_nextId++;
    }

    uint32_t m_nextId = 0;
};

} // namespace BitFlow::Core::Testing::SHA
