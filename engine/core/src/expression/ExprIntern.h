#pragma once

#include "expression/ExprKey.h"
#include "expression/ExprKeyHash.h"

#include <BitFlow/core/expression/Expression.h>
#include <unordered_map>

namespace BitFlow::Core::Expression {

class ExprIntern {
  public:
    using Key = Expression::ExprKey;
    using KeyHash = Expression::ExprKeyHash;
    using KeyBuilderFn = Key (*)(const ExprOld*);

  public:
    static ExprOld* Intern(ExprOld* e) {
        if (e->id.value() == 0)
            e->id = Ids::ExprId{NextId()};

        Key key = GetKeyBuilder()(e);

        auto it = Table().find(key);
        if (it != Table().end())
            return it->second;

        Table()[key] = e;
        return e;
    }

    static void Clear() {
        Table().clear();
    }

    static void Reset() {
        Clear();
        ResetKeyBuilder();
        ResetIds();
    }

    static void SetKeyBuilder(KeyBuilderFn fn) {
        GetKeyBuilder() = fn ? fn : &BuildStructuralKey;
    }

    static void ResetKeyBuilder() {
        GetKeyBuilder() = &BuildStructuralKey;
    }

  private:
    static std::unordered_map<Key, ExprOld*, KeyHash>& Table() {
        static std::unordered_map<Key, ExprOld*, KeyHash> table;
        return table;
    }

    static KeyBuilderFn& GetKeyBuilder() {
        static KeyBuilderFn fn = &BuildStructuralKey;
        return fn;
    }

    static uint32_t& NextIdRef() {
        static uint32_t nextId = 1000000;
        return nextId;
    }

    static uint32_t NextId() {
        return NextIdRef()++;
    }

    static void ResetIds() {
        NextIdRef() = 1000000;
    }

    static Key BuildStructuralKey(const ExprOld* e) {
        Key k{};
        k.op = e->op;
        k.constValue = e->constValue;

        k.inputs.reserve(e->inputs.size());

        for (const ExprOld* in : e->inputs)
            k.inputs.push_back(in->id.value());

        if (e->inputs.empty()) {
            if (e->op == OpType::Var)
                k.inputs.push_back(e->id.value());
            else if (e->op == OpType::Const)
                k.inputs.push_back(e->constValue);
        }

        return k;
    }
};

} // namespace BitFlow::Core::Expression
