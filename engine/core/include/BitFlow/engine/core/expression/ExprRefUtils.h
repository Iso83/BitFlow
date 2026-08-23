#pragma once

#include <BitFlow/engine/core/expression/ExprStore.h>

namespace BitFlow::Engine::Core::Expression {

inline bool EqualChunkValue(const ExprStore* store, const Ids::ExprId id, const Types::ExprChunk& value) {
    const Expr& expr = (*store)[id];
    return expr.op == OpType::Const && expr.inputs.empty() && expr.knownValue == value;
}

inline bool EqualChunkValue(const ExprRef& e, const Types::ExprChunk& value) {
    return EqualChunkValue(e.store, e.id, value);
}

} // namespace BitFlow::Core::Expression
