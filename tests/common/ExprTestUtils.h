#pragma once

#include <BitFlow/core/eval/Evaluator.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <cassert>
#include <cstdint>
#include <iostream>

#ifdef USE_CORE_PRINT
#include <BitFlow/core/expression/ExprPrinter.h>
#else
#include <BitFlow/io/ExprParser.h>
#endif

namespace BitFlow::Testing {

inline const Core::Expression::Expr& ExprOf(const Core::Expression::ExprRef e) {
    return (*e.store)[e];
}

inline Core::Expression::ExprRef Input(const Core::Expression::ExprRef e, std::size_t index) {
    return Core::Expression::ExprRef(e.store, ExprOf(e).inputs[index]);
}

inline auto InputSize(const Core::Expression::ExprRef e) {
    return ExprOf(e).inputs.size();
}

inline auto BitWidth(const Core::Expression::ExprRef e) {
    return ExprOf(e).bitWidth;
}

inline Core::Expression::OpType Op(const Core::Expression::ExprRef e) {
    return (*e.store)[e].op;
}

template <typename Pred> inline bool AnyInput(Core::Expression::ExprRef e, Pred&& pred) {
    const Core::Expression::Expr& expr = (*e.store)[e];

    for (auto in : expr.inputs) {
        if (pred(Core::Expression::ExprRef(e.store, in)))
            return true;
    }

    return false;
}

template <typename Fn> inline size_t CountInput(Core::Expression::ExprRef e, Fn&& fn) {
    size_t count = 0;

    const Core::Expression::Expr& expr = (*e.store)[e];

    for (auto in : expr.inputs)
        count += fn(Core::Expression::ExprRef(e.store, in));

    return count;
}

inline size_t CountExpr(Core::Expression::ExprRef e, Core::Expression::ExprRef t) {
    return CountInput(e, [&](Core::Expression::ExprRef in) { return in == t; });
}

#pragma region Evaluate
inline bool EqualChunkValue(const Core::BitVector::bf_uint& a, const Core::BitVector::bf_uint& b) {
    const Core::Types::BitWidth bw = std::max(a.BitWidth(), b.BitWidth());

    Core::BitVector::bf_uint aa = a;
    Core::BitVector::bf_uint bb = b;

    aa = Core::BitVector::bf_uint(aa.ToChunk(), bw);
    bb = Core::BitVector::bf_uint(bb.ToChunk(), bw);

    return aa == bb;
}

inline bool EqualChunkValue(const Core::BitVector::bf_uint& a, const Core::Types::ExprChunk& b) {
    return EqualChunkValue(a, Core::BitVector::bf_uint(b, Core::Types::ExprChunkBits));
}

inline bool EqualChunkValue(const Core::Expression::ExprStore* store, const Core::Ids::ExprId id,
                            const Core::Types::ExprChunk& value) {
    const Core::Expression::Expr& expr = (*store)[id];

    return expr.op == Core::Expression::OpType::Const && expr.inputs.empty() && expr.knownValue == value;
}

inline bool EqualChunkValue(const Core::Expression::ExprRef e, const Core::Types::ExprChunk& value) {
    return EqualChunkValue(e.store, e.id, value);
}

inline Core::Eval::EvalResult EvaluateConstant(const Core::Expression::ExprRef root,
                                               Core::Types::BitWidth bitWidth = Core::Types::ExprChunkBits) {
    _ASSERT(root.IsValid());
    return Core::Eval::EvaluateConstant(root.store, &(*root.store)[root], bitWidth);
}

inline bool IsFullyConstant(const Core::Expression::ExprRef root) {
    assert(root.IsValid());
    return Core::Eval::IsFullyConstant(root.store, &(*root.store)[root.id]);
}
#pragma endregion

#pragma region Test DSL

#ifndef USE_CORE_PRINT
#define LAMBDA_IO_PRASE auto Parse = [&](const std::string& input) { return BitFlow::IO::Parse(&store, input); };
#else
#define LAMBDA_IO_PRASE
#endif

#define MakeExprStore(bw)                                                                                              \
    BitFlow::Core::Expression::ExprStore store;                                                                        \
    auto C = [&](uint64_t v) { return store.createConstant(v, bw); };                                                  \
    BitFlow::Core::Expression::ExprNameMap names;                                                                      \
    auto V = [&](const std::string name = "") {                                                                        \
        auto e = store.createVariable(bw);                                                                             \
        if (!name.empty())                                                                                             \
            names.emplace(e.id, name);                                                                                 \
        return e;                                                                                                      \
    };                                                                                                                 \
    auto Eval = [&](const BitFlow::Core::Expression::ExprRef root, BitFlow::Core::Types::ExprChunk v,                  \
                    BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                    \
        return EqualChunkValue(EvaluateConstant(root, bw).value, v);                                                   \
    };                                                                                                                 \
    auto ToString = [&](const BitFlow::Core::Expression::ExprRef root,                                                 \
                        const BitFlow::Core::Expression::PrintOptions& options =                                       \
                            BitFlow::Core::Expression::PrintOptions{},                                                 \
                        const BitFlow::Core::Expression::ExprNameMap& _names = {}) {                                   \
        return BitFlow::Core::Expression::ToString(root.store, root.id, _names.empty() ? names : _names, options);     \
    };                                                                                                                 \
    auto False = [&](BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                   \
        return ExprRef(&store, store.makeFalse(bitWidth).id);                                                          \
    };                                                                                                                 \
    auto True = [&](BitFlow::Core::Types::BitWidth bitWidth = bw) {                                                    \
        return ExprRef(&store, store.makeTrue(bitWidth).id);                                                           \
    };                                                                                                                 \
    LAMBDA_IO_PRASE

#define E(name, expr) V(#name) = expr

#pragma endregion

inline bool IsFalse(const Core::Expression::ExprRef e) {
    return e.store->isFalse(e.id);
}

inline bool IsTrue(const Core::Expression::ExprRef e) {
    return e.store->isTrue(e.id);
}

inline bool Contains(const std::string& s, const char* text) {
    return s.find(text) != std::string::npos;
}

inline bool Contains(const std::string& s, char ch) {
    return s.find(ch) != std::string::npos;
}

} // namespace BitFlow::Testing
