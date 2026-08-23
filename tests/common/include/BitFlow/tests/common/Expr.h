#pragma once

#include <BitFlow/engine/core/eval/Evaluator.h>
#include <BitFlow/engine/core/expression/ExprRefUtils.h>

#ifdef USE_CORE_PRINT
#include <BitFlow/engine/core/expression/ExprPrinter.h>
#else
#include <BitFlow/engine/io/ExprParser.h>
#endif

namespace BitFlow::Testing {

namespace Expression = Engine::Core::Expression;
namespace Types = Engine::Core::Types;
namespace BitVector = Engine::Core::BitVector;
namespace Eval = Engine::Core::Eval;

#pragma region ExprRef Access
inline const Expression::Expr* ExprPtr(const Expression::ExprRef e) {
    return &(*e.store)[e];
}

inline const Expression::Expr& ExprOf(const Expression::ExprRef e) {
    return *ExprPtr(e);
}

inline Types::BitWidth BitWidth(const Expression::ExprRef e) {
    return ExprPtr(e)->bitWidth;
}

inline Expression::OpType Op(const Expression::ExprRef e) {
    return ExprPtr(e)->op;
}
#pragma endregion

#pragma region ExprRef Inputs
inline Expression::ExprRef Input(const Expression::ExprRef e, std::size_t index) {
    return Expression::ExprRef(e.store, ExprPtr(e)->inputs[index]);
}

inline std::size_t InputSize(const Expression::ExprRef e) {
    return ExprPtr(e)->inputs.size();
}

template <typename Pred> inline bool AnyInput(const Expression::ExprRef e, Pred&& pred) {
    for (auto id : ExprPtr(e)->inputs) {
        if (pred(Expression::ExprRef(e.store, id)))
            return true;
    }

    return false;
}

template <typename Pred> inline std::size_t CountInputsIf(const Expression::ExprRef e, Pred&& pred) {
    std::size_t count = 0;

    for (auto id : ExprPtr(e)->inputs) {
        if (pred(Expression::ExprRef(e.store, id)))
            ++count;
    }

    return count;
}

inline std::size_t CountInput(const Expression::ExprRef e, const Expression::ExprRef target) {
    return CountInputsIf(e, [&](const Expression::ExprRef in) { return in == target; });
}
#pragma endregion

#pragma region ExprRef Predicates
inline bool IsFalse(const Expression::ExprRef e) {
    return e.store->isFalse(e.id);
}

inline bool IsTrue(const Expression::ExprRef e) {
    return e.store->isTrue(e.id);
}

inline bool IsPow(const Expression::ExprRef e, const Expression::ExprRef base, const Expression::ExprRef exponent) {
    return Op(e) == Expression::OpType::Pow && InputSize(e) == 2 && Input(e, 0) == base && Input(e, 1) == exponent;
}

inline bool IsPow(const Expression::ExprRef e, const Expression::ExprRef base, Types::ExprChunk exponent) {
    return Op(e) == Expression::OpType::Pow && InputSize(e) == 2 && Input(e, 0) == base &&
           EqualChunkValue(Input(e, 1), exponent);
}
#pragma endregion

#pragma region BitVector Compare
inline bool EqualChunkValue(const BitVector::bf_uint& a, const BitVector::bf_uint& b) {
    const Types::BitWidth bitWidth = std::max(a.BitWidth(), b.BitWidth());

    const BitVector::bf_uint aa(a.ToChunk(), bitWidth);
    const BitVector::bf_uint bb(b.ToChunk(), bitWidth);

    return aa == bb;
}

inline bool EqualChunkValue(const BitVector::bf_uint& a, Types::ExprChunk b) {
    return EqualChunkValue(a, BitVector::bf_uint(b, Types::ExprChunkBits));
}
#pragma endregion

#pragma region Evaluate
inline Eval::EvalResult EvaluateConstant(const Expression::ExprRef root,
                                         Types::BitWidth bitWidth = Types::ExprChunkBits) {

    assert(root.IsValid());
    return Eval::EvaluateConstant(root.store, ExprPtr(root), bitWidth);
}

inline bool IsFullyConstant(const Expression::ExprRef root) {
    assert(root.IsValid());
    return Eval::IsFullyConstant(root.store, ExprPtr(root));
}
#pragma endregion

#pragma region CTest DSL

#ifndef USE_CORE_PRINT
#define LAMBDA_IO_PARSE                                                                                                \
    auto Parse = [&](const std::string& input) { return BitFlow::Engine::IO::Parse(&store, input); };
#else
#define LAMBDA_IO_PARSE
#endif

#define MakeExprStore(bw)                                                                                              \
    BitFlow::Engine::Core::Expression::ExprStore store;                                                                \
    const BitFlow::Engine::Core::Types::BitWidth bitWidth(bw);                                                         \
    auto C = [&](uint64_t v) { return store.createConstant(v, bw); };                                                  \
    BitFlow::Engine::Core::Expression::ExprNameMap names;                                                              \
    auto V = [&](const std::string name = "") {                                                                        \
        auto e = store.createVariable(bw);                                                                             \
        if (!name.empty())                                                                                             \
            names.emplace(e.id, name);                                                                                 \
        return e;                                                                                                      \
    };                                                                                                                 \
    auto Eval = [&](const BitFlow::Engine::Core::Expression::ExprRef root,                                             \
                    BitFlow::Engine::Core::Types::ExprChunk value,                                                     \
                    BitFlow::Engine::Core::Types::BitWidth evalBitWidth = bw) {                                        \
        return EqualChunkValue(EvaluateConstant(root, evalBitWidth).value, value);                                     \
    };                                                                                                                 \
    auto ToString = [&](const BitFlow::Engine::Core::Expression::ExprRef root,                                         \
                        const BitFlow::Engine::Core::Expression::PrintOptions& options =                               \
                            BitFlow::Engine::Core::Expression::PrintOptions{},                                         \
                        const BitFlow::Engine::Core::Expression::ExprNameMap& _names = {}) {                           \
        return BitFlow::Engine::Core::Expression::ToString(root.store, root.id, _names.empty() ? names : _names,       \
                                                           options);                                                   \
    };                                                                                                                 \
    auto False = [&](BitFlow::Engine::Core::Types::BitWidth valueBitWidth = bw) {                                      \
        return BitFlow::Engine::Core::Expression::ExprRef(&store, store.makeFalse(valueBitWidth).id);                  \
    };                                                                                                                 \
    auto True = [&](BitFlow::Engine::Core::Types::BitWidth valueBitWidth = bw) {                                       \
        return BitFlow::Engine::Core::Expression::ExprRef(&store, store.makeTrue(valueBitWidth).id);                   \
    };                                                                                                                 \
    LAMBDA_IO_PARSE

#define E(name, expr) V(#name) = expr

#pragma endregion
} // namespace BitFlow::Testing
