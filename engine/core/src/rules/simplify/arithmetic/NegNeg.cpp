#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Neg_Neg(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Neg || e.inputs.size() != 1)
        return false;

    const Expr& in = store->get(e.inputs[0]);
    return in.op == OpType::Neg && in.inputs.size() == 1;
}

static ExprId Rewrite_Neg_Neg(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    _ASSERT(e.op == OpType::Not && e.inputs.size() == 1);

    const Expr& e2 = store->get(e.inputs[0]);
    _ASSERT(e2.op == OpType::Not && e2.inputs.size() == 1);

    return e2.inputs[0];
}

Rule Get_Neg_Neg_Rule() {
    return Rule{Neg_Neg, &Match_Neg_Neg, &Rewrite_Neg_Neg, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
