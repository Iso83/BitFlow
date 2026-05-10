#pragma once

#include <BitFlow/core/eval/Evaluator.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/helper/Attributes.h>
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

#pragma region Evaluate
inline bool EqualBits(const BitVector::bf_uint& a, const BitVector::bf_uint& b) {
    const Types::BitWidth bw = std::max(a.BitWidth(), b.BitWidth());

    BitVector::bf_uint aa = a;
    BitVector::bf_uint bb = b;

    aa = BitVector::bf_uint(aa.ToChunk(), bw);
    bb = BitVector::bf_uint(bb.ToChunk(), bw);

    return aa == bb;
}

inline bool EqualBits(const BitVector::bf_uint& a, const Types::ExprChunk& b) {
    return EqualBits(a, BitVector::bf_uint(b, Types::ExprChunkBits));
}

inline Eval::EvalResult EvaluateConstant(const Expression::ExprRef root, Types::BitWidth bitWidth = 32) {
    assert(root.IsValid());
    return Eval::EvaluateConstant(root.store, &root.store->get(root.id), bitWidth);
}

inline bool IsFullyConstant(const Expression::ExprRef root) {
    assert(root.IsValid());
    return Eval::IsFullyConstant(root.store, &root.store->get(root.id));
}
#pragma endregion

#if USE_CORE_PRINT
#define UseExprPrint BitFlow::Core::Expression::ToString(root.store, root.id, names, options)
#else
#define UseExprPrint std::string
#endif

#pragma region Local: fields & fn
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
        return EqualBits(EvaluateConstant(root).value, v);                                                             \
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

#pragma region RuleEngine
inline Expression::ExprRef Rewrite(Expression::ExprStore* store, Rules::RuleEngine& engine, Ids::ExprId id) {
    return Expression::ExprRef(store, engine.Rewrite(store, id));
}

inline Expression::ExprRef
Rewrite(Rules::RuleEngine& engine, Expression::ExprRef e,
        const std::unordered_map<BitFlow::Core::Ids::ExprId, std::string>* traceNames = nullptr,
        const Expression::PrintOptions& options = {}) {

    if (traceNames == nullptr)
        return Rewrite(e.store, engine, e.id);

    std::cout << "=== Rewrite Trace ===" << std::endl;
    std::cout << "Input : " << ToString(e.store, e.id, *traceNames, options) << std::endl;

    engine.SetDebugCallback(
        [traceNames, step = 0, e, options](Ids::ExprId before, Ids::ExprId after, Rules::RuleKey key) mutable {
            if (before == after)
                return;

            std::cout << "#" << step++ << " [" << key.value << "] " << ToString(e.store, before, *traceNames, options)
                      << " -> " << ToString(e.store, after, *traceNames, options) << std::endl;
        });

    auto result = Rewrite(e.store, engine, e.id);

    engine.SetDebugCallback(nullptr);

    std::cout << "Result: " << ToString(result.store, result.id, *traceNames, options) << std::endl;
    std::cout << std::endl;

    return result;
}

inline const Expression::Expr& GetExpr(const Expression::ExprRef e) {
    return e.store->get(e.id);
}

inline bool IsConstantValue(const Expression::ExprStore* store, const Ids::ExprId id, const uint64_t value) {
    const Expression::Expr& expr = store->get(id);

    return expr.op == Expression::OpType::Const && expr.inputs.empty() && expr.knownValue == value;
}

inline bool IsConstantValue(const Expression::ExprRef e, const uint64_t value) {
    return IsConstantValue(e.store, e.id, value);
}

inline size_t CountExpr(const Expression::ExprStore* store, Ids::ExprId id, Ids::ExprId target) {
    size_t count = 0;

    if (id == target)
        ++count;

    const auto& e = store->get(id);

    for (auto in : e.inputs) {
        if (in == target)
            ++count;
    }

    return count;
}

inline size_t CountExpr(const Expression::ExprRef e, const Expression::ExprRef t) {
    if (e.store != t.store)
        return 0;

    return CountExpr(e.store, e.id, t.id);
}

template <typename Pred> inline bool AnyInput(Expression::ExprRef e, Pred&& pred) {
    const auto& expr = e.store->get(e.id);

    for (auto in : expr.inputs) {
        if (pred(Expression::ExprRef(e.store, in)))
            return true;
    }

    return false;
}

inline bool IsFalse(const Expression::ExprStore* store, Ids::ExprId id) {
    const Expression::Expr& e = store->get(id);
    return e.op == Expression::OpType::Const && e.inputs.empty() && e.knownValue == 0;
}

inline bool IsFalse(const Expression::ExprRef e) {
    return IsFalse(e.store, e.id);
}

inline bool IsTrue(const Expression::ExprStore* store, Ids::ExprId id) {
    const Expression::Expr& e = store->get(id);
    return e.op == Expression::OpType::Const && e.inputs.empty() &&
           e.knownValue == Expression::Expr::fullMask(e.bitWidth);
}

inline bool IsTrue(const Expression::ExprRef e) {
    return IsTrue(e.store, e.id);
}
#pragma endregion

} // namespace BitFlow::Core::Testing
