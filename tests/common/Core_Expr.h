#pragma once

#include <BitFlow/core/eval/Evaluator.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/helper/Attributes.h>
#include <BitFlow/core/ids/ExprId.h>
#include <cassert>
#include <cstdint>

#ifdef USE_CORE_PRINT
#include "expression/ExprPrinter.h"
#else
// IO ToString
#endif

namespace BitFlow::Core::Testing {

inline bool EqualBits(const BitVector::bf_uint& a, const BitVector::bf_uint& b) {
    const uint32_t bw = std::max(a.BitWidth(), b.BitWidth());

    BitVector::bf_uint aa = a;
    BitVector::bf_uint bb = b;

    aa = BitVector::bf_uint(aa.ToUint64(), bw);
    bb = BitVector::bf_uint(bb.ToUint64(), bw);

    return aa == bb;
}

inline bool EqualBits(const BitVector::bf_uint& a, const uint64_t& b) {
    return EqualBits(a, BitVector::bf_uint(b, 64));
}

inline Eval::EvalResult EvaluateConstant(const Expression::ExprRef root, uint32_t bitWidth = 32) {
    assert(root.IsValid());
    return Eval::EvaluateConstant(root.store, &root.store->get(root.id), bitWidth);
}

inline bool IsFullyConstant(const Expression::ExprRef root) {
    assert(root.IsValid());
    return Eval::IsFullyConstant(root.store, &root.store->get(root.id));
}

#if USE_CORE_PRINT
#define UseExprPrint BitFlow::Core::Expression::ToString(root.store, root.id, names, options)
#else
#define UseExprPrint std::string
#endif

#define MakeExprStore(bw)                                                                                              \
    ExprStore eStore;                                                                                                  \
    auto C = [&](uint64_t v) { return eStore.createConstant(v, bw); };                                                 \
    auto V = [&]() { return eStore.createVariable(bw); };                                                              \
    auto Eval = [&](const BitFlow::Core::Expression::ExprRef root, uint64_t v, uint32_t bitWidth = bw) {               \
        return EqualBits(EvaluateConstant(root).value, v);                                                             \
    };                                                                                                                 \
    auto ToString = [&](const BitFlow::Core::Expression::ExprRef root,                                                 \
                        const std::unordered_map<BitFlow::Core::Ids::ExprId, std::string> names = {},                  \
                        const BitFlow::Core::Expression::PrintOptions& options =                                       \
                            BitFlow::Core::Expression::PrintOptions{}) { return UseExprPrint; };

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeVar(uint32_t id) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Var;
    return e;
}

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeOp(uint32_t id, Expression::OpType op, std::initializer_list<Expression::ExprOld*> in) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeConst(uint32_t id, uint32_t v) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Const;
    e->constValue = v;
    return e;
}

} // namespace BitFlow::Core::Testing
