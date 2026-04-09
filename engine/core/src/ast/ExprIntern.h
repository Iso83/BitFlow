#pragma once

#include "expression/ExprKey.h"
#include "expression/ExprKeyHash.h"
#include <BitFlow/core/ast/Expression.h>

#include <unordered_map>

namespace BitFlow::Core::AST {

class ExprIntern {
  public:
    static Expr* Intern(Expr* e) {
        Expression::ExprKey key = BuildKey(e);

        auto& map = storage();
        auto it = map.find(key);
        if (it != map.end())
            return it->second;

        e->frozen = true;

        map[key] = e;
        return e;
    }

  private:
    static Expression::ExprKey BuildKey(const Expr* e) {
        Expression::ExprKey k;
        k.op = e->op;
        k.isConst = e->isConst;
        k.constValue = e->constValue;

        for (const Expr* in : e->inputs) {
            k.inputs.push_back(in->id.value());
        }

        if (!e->isConst && e->inputs.empty()) {
            k.hasSymbolId = true;
            k.symbolId = e->id.value();
        }

        return k;
    }

    static std::unordered_map<Expression::ExprKey, Expr*, Expression::ExprKeyHash>& storage() {
        static std::unordered_map<Expression::ExprKey, Expr*, Expression::ExprKeyHash> s;
        return s;
    }
};

} // namespace BitFlow::Core::AST