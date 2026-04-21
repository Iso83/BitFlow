#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <cstddef>
#include <unordered_map>

namespace BitFlow::Core::AST {

inline Expr* Clone(const Expr* expr) {
    std::unordered_map<const Expr*, Expr*> cloned{};

    const auto cloneNode = [&](const auto& self, const Expr* node) -> Expr* {
        auto it = cloned.find(node);
        if (it != cloned.end())
            return it->second;

        Expr* out = new Expr{};
        out->id = node->id;
        out->op = node->op;
        out->constValue = node->constValue;
        out->frozen = node->frozen;
        cloned.emplace(node, out);

        out->inputs.reserve(node->inputs.size());
        for (const Expr* input : node->inputs) {
            out->inputs.push_back(self(self, input));
        }

        return out;
    };

    return cloneNode(cloneNode, expr);
}

inline bool StructEqual(const Expr* a, const Expr* b) {
    std::unordered_map<const Expr*, const Expr*> seen{};

    const auto eqNode = [&](const auto& self, const Expr* lhs, const Expr* rhs) -> bool {
        if (lhs == rhs)
            return true;
        if (!lhs || !rhs)
            return lhs == rhs;

        auto seenIt = seen.find(lhs);
        if (seenIt != seen.end())
            return seenIt->second == rhs;

        if (lhs->op != rhs->op || lhs->constValue != rhs->constValue || lhs->inputs.size() != rhs->inputs.size())
            return false;

        seen.emplace(lhs, rhs);
        for (size_t i = 0; i < lhs->inputs.size(); ++i) {
            if (!self(self, lhs->inputs[i], rhs->inputs[i]))
                return false;
        }

        return true;
    };

    return eqNode(eqNode, a, b);
}

} // namespace BitFlow::Core::AST
