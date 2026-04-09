#pragma once

#include <BitFlow/core/ExprKey.h>
#include <BitFlow/core/ExprKeyHash.h>
#include <BitFlow/core/Expression.h>
#include <unordered_map>

namespace BitFlow::Core {

class ExprIntern {
  public:
    static Expr* Intern(Expr* e) {
        ExprKey key = BuildKey(e);

        auto& map = storage();
        auto it = map.find(key);
        if (it != map.end()) {
            return it->second;
        }

        map[key] = e;
        return e;
    }

  private:
    static ExprKey BuildKey(const Expr* e) {
        ExprKey k;
        k.op = e->op;
        k.isConst = e->isConst;
        k.constValue = e->constValue;

        for (const Expr* in : e->inputs) {
            k.inputs.push_back(in->id.value());
        }

        // symbolische leafs moeten unieke identity behouden
        if (!e->isConst && e->inputs.empty()) {
            k.hasSymbolId = true;
            k.symbolId = e->id.value();
        }

        return k;
    }

    static std::unordered_map<ExprKey, Expr*, ExprKeyHash>& storage() {
        static std::unordered_map<ExprKey, Expr*, ExprKeyHash> s;
        return s;
    }
};

} // namespace BitFlow::Core