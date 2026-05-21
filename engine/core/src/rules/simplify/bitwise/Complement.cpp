#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool IsNotOf(const ExprStore* store, ExprId a, ExprId b) {
    const Expr& exprA = (*store)[a];
    return exprA.op == OpType::Not && exprA.inputs.size() == 1 && store->structuralEquivalent(exprA.inputs[0], b);
}

static bool Match_Complement(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if ((e.op != OpType::And && e.op != OpType::Or) || e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            ExprId a = e.inputs[i];
            ExprId b = e.inputs[j];

            if (IsNotOf(store, a, b) || IsNotOf(store, b, a))
                return true;
        }
    }

    return false;
}

static ExprId Rewrite_Complement(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            ExprId a = e.inputs[i];
            ExprId b = e.inputs[j];

            if (IsNotOf(store, a, b) || IsNotOf(store, b, a)) {
                if (e.op == OpType::And)
                    return store->makeFalse().id;

                if (e.op == OpType::Or)
                    return store->makeTrue().id;
            }
        }
    }

    BF_CORE_ASSERT(false);
    return id;
}

Rule Get_Complement_Rule() {
    return Rule{Complement, &Match_Complement, &Rewrite_Complement, {Idempotent}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise