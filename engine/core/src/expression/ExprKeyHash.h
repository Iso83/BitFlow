#pragma once

#include <vector>

namespace BitFlow::Core ::Expression {

struct ExprKey;

struct ExprKeyHash {
    std::size_t operator()(const ExprKey& k) const;
};

} // namespace BitFlow::Core::Expression