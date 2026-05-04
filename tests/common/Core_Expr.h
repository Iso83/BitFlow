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
    ExprStore store;                                                                                                   \
    auto C = [&](uint64_t v) { return store.createConstant(v, bw); };                                                  \
    auto V = [&]() { return store.createVariable(bw); };                                                               \
    auto Eval = [&](const BitFlow::Core::Expression::ExprRef root, uint64_t v, uint32_t bitWidth = bw) {               \
        return EqualBits(EvaluateConstant(root).value, v);                                                             \
    };                                                                                                                 \
    auto ToString = [&](const BitFlow::Core::Expression::ExprRef root,                                                 \
                        const std::unordered_map<BitFlow::Core::Ids::ExprId, std::string> names = {},                  \
                        const BitFlow::Core::Expression::PrintOptions& options =                                       \
                            BitFlow::Core::Expression::PrintOptions{}) { return UseExprPrint; };

} // namespace BitFlow::Core::Testing
