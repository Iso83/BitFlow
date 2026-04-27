#pragma once

#include <BitFlow/core/expression/Expression.h>
#include <cstddef>
#include <unordered_map>

namespace BitFlow::Core::Expression {

// fout gebruik bij rewrite --> opruimen als Expr.ID overral gebruikt wordt.
inline Expr* CloneExpr(const Expr* e) {
    Expr* n = new Expr{};
    n->op = e->op;
    n->constValue = e->constValue;
    n->inputs = e->inputs;
    return n;
}

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

class ConstPool {
  public:
    static Expr* Get(uint32_t value) {
        auto it = pool().find(value);
        if (it != pool().end())
            return it->second;

        Expr* e = new Expr{};
        e->op = OpType::Const;
        e->constValue = value;
        e->id = Ids::ExprId{NextId()};

        pool()[value] = e;
        return e;
    }

  private:
    static std::unordered_map<uint32_t, Expr*>& pool() {
        static std::unordered_map<uint32_t, Expr*> p;
        return p;
    }

    static uint32_t NextId() {
        static uint32_t id = 1000000;
        return id++;
    }
};
} // namespace BitFlow::Core::Expression