#pragma once

#include <BitFlow/core/eval/Evaluator.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <cassert>
#include <cstdint>
#include <iostream>

#ifdef USE_CORE_PRINT
#include "expression/ExprPrinter.h"
#else
// IO ToString
#endif

namespace BitFlow::Core::Testing {

inline const Expression::Expr& GetExpr(const Expression::ExprRef e) {
    return (*e.store)[e];
}

template <typename Pred> inline bool AnyInput(Expression::ExprRef e, Pred&& pred) {
    const Expression::Expr& expr = (*e.store)[e];

    for (auto in : expr.inputs) {
        if (pred(Expression::ExprRef(e.store, in)))
            return true;
    }

    return false;
}

template <typename Fn> inline size_t CountInput(Expression::ExprRef e, Fn&& fn) {
    size_t count = 0;

    const Expression::Expr& expr = (*e.store)[e];

    for (auto in : expr.inputs)
        count += fn(Expression::ExprRef(e.store, in));

    return count;
}

inline size_t CountExpr(Expression::ExprRef e, Expression::ExprRef t) {
    return CountInput(e, [&](Expression::ExprRef in) { return in == t; });
}

#pragma region Evaluate
inline bool EqualChunkValue(const BitVector::bf_uint& a, const BitVector::bf_uint& b) {
    const Types::BitWidth bw = std::max(a.BitWidth(), b.BitWidth());

    BitVector::bf_uint aa = a;
    BitVector::bf_uint bb = b;

    aa = BitVector::bf_uint(aa.ToChunk(), bw);
    bb = BitVector::bf_uint(bb.ToChunk(), bw);

    return aa == bb;
}

inline bool EqualChunkValue(const BitVector::bf_uint& a, const Types::ExprChunk& b) {
    return EqualChunkValue(a, BitVector::bf_uint(b, Types::ExprChunkBits));
}

inline bool EqualChunkValue(const Expression::ExprStore* store, const Ids::ExprId id, const Types::ExprChunk& value) {
    const Expression::Expr& expr = (*store)[id];

    return expr.op == Expression::OpType::Const && expr.inputs.empty() && expr.knownValue == value;
}

inline bool EqualChunkValue(const Expression::ExprRef e, const Types::ExprChunk& value) {
    return EqualChunkValue(e.store, e.id, value);
}

inline Eval::EvalResult EvaluateConstant(const Expression::ExprRef root,
                                         Types::BitWidth bitWidth = Types::ExprChunkBits) {
    assert(root.IsValid());
    return Eval::EvaluateConstant(root.store, &(*root.store)[root], bitWidth);
}

inline bool IsFullyConstant(const Expression::ExprRef root) {
    assert(root.IsValid());
    return Eval::IsFullyConstant(root.store, &(*root.store)[root.id]);
}
#pragma endregion

#pragma region Test DSL
#if USE_CORE_PRINT
#define UseExprPrint BitFlow::Core::Expression::ToString(root.store, root.id, names, options)
#else
#define UseExprPrint std::string
#endif

#define MakeExprStore(bw)                                                                                              \
    ExprStore store;                                                                                                   \
    auto C = [&](uint64_t v) { return store.createConstant(v, bw); };                                                  \
    std::unordered_map<BitFlow::Core::Ids::ExprId, std::string> names;                                                 \
    auto V = [&](const std::string name = "") {                                                                        \
        auto e = store.createVariable(bw);                                                                             \
        if (!name.empty())                                                                                             \
            names.emplace(e.id, name);                                                                                 \
        return e;                                                                                                      \
    };                                                                                                                 \
    auto Eval = [&](const BitFlow::Core::Expression::ExprRef root, BitFlow::Core::Types::ExprChunk v,                  \
                    BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                    \
        return EqualChunkValue(EvaluateConstant(root).value, v);                                                       \
    };                                                                                                                 \
    auto ToString = [&](const BitFlow::Core::Expression::ExprRef root,                                                 \
                        const std::unordered_map<BitFlow::Core::Ids::ExprId, std::string> names = {},                  \
                        const BitFlow::Core::Expression::PrintOptions& options =                                       \
                            BitFlow::Core::Expression::PrintOptions{}) { return UseExprPrint; };                       \
    auto False = [&](BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                   \
        return ExprRef(&store, store.makeFalse(bitWidth).id);                                                          \
    };                                                                                                                 \
    auto True = [&](BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                    \
        return ExprRef(&store, store.makeTrue(bitWidth).id);                                                           \
    };

#define E(name, expr) V(#name) = expr

#define ERef(id) ExprRef(&store, id)
#pragma endregion

inline bool IsFalse(const Expression::ExprRef e) {
    return e.store->isFalse(e.id);
}

inline bool IsTrue(const Expression::ExprRef e) {
    return e.store->isTrue(e.id);
}

} // namespace BitFlow::Core::Testing
