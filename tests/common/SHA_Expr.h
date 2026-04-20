#pragma once

#include <Core_Expr.h>
#include <cstdint>
#include <initializer_list>

namespace BitFlow::Core::Testing::SHA {

class Builder {
  public:
    explicit Builder(uint32_t nextId = 1000) : m_nextId(nextId) {}

    Expr* Var() {
        return MakeVar(NextId());
    }

    Expr* Const(uint32_t value) {
        return MakeConst(NextId(), value);
    }

    Expr* Not(Expr* x) {
        return MakeOp(NextId(), OpType::Not, {x});
    }

    Expr* And(Expr* a, Expr* b) {
        return MakeOp(NextId(), OpType::And, {a, b});
    }

    Expr* Xor(std::initializer_list<Expr*> terms) {
        return MakeOp(NextId(), OpType::Xor, terms);
    }

    Expr* Add(std::initializer_list<Expr*> terms) {
        return MakeOp(NextId(), OpType::Add, terms);
    }

    Expr* RotR(Expr* x, uint32_t amount) {
        return MakeOp(NextId(), OpType::RotR, {x, Const(amount)});
    }

    Expr* RotL(Expr* x, uint32_t amount) {
        return MakeOp(NextId(), OpType::RotL, {x, Const(amount)});
    }

    Expr* Ch(Expr* x, Expr* y, Expr* z) {
        return MakeOp(NextId(), OpType::Ch, {x, y, z});
    }

    Expr* Maj(Expr* x, Expr* y, Expr* z) {
        return MakeOp(NextId(), OpType::Maj, {x, y, z});
    }

    // SHA-256 upper-case sigma helpers.
    Expr* BigSigma0(Expr* x) {
        return Xor({RotR(x, 2), RotR(x, 13), RotR(x, 22)});
    }

    Expr* BigSigma1(Expr* x) {
        return Xor({RotR(x, 6), RotR(x, 11), RotR(x, 25)});
    }

    // SHA-256 lower-case sigma helpers.
    Expr* SmallSigma0(Expr* x) {
        return Xor({RotR(x, 7), RotR(x, 18), MakeOp(NextId(), OpType::Shr, {x, Const(3)})});
    }

    Expr* SmallSigma1(Expr* x) {
        return Xor({RotR(x, 17), RotR(x, 19), MakeOp(NextId(), OpType::Shr, {x, Const(10)})});
    }

    Expr* RoundT1ChoiceCore(Expr* e, Expr* f, Expr* g) {
        return Ch(e, f, g);
    }

    Expr* RoundT1SigmaChoiceCore(Expr* e, Expr* f, Expr* g) {
        return Add({BigSigma1(e), RoundT1ChoiceCore(e, f, g)});
    }

    Expr* RoundT1PartCore(Expr* h, Expr* e, Expr* f, Expr* g) {
        return Add({h, RoundT1SigmaChoiceCore(e, f, g)});
    }

    Expr* RoundT2PartCore(Expr* a, Expr* b, Expr* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

    // Typical SHA-256 round fragments.
    Expr* RoundT1(Expr* h, Expr* e, Expr* f, Expr* g, Expr* k, Expr* w) {
        return Add({h, BigSigma1(e), Ch(e, f, g), k, w});
    }

    Expr* RoundT2(Expr* a, Expr* b, Expr* c) {
        return Add({BigSigma0(a), Maj(a, b, c)});
    }

  private:
    uint32_t NextId() {
        return m_nextId++;
    }

    uint32_t m_nextId = 0;
};

} // namespace BitFlow::Core::Testing::SHA
