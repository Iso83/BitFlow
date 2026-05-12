#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static ExprId Rewrite_RemoveZero(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

Rule Get_XorZero_Rule() {
    return Rule{XorZero, &Match_Zero<OpType::Xor>, &Rewrite_RemoveZero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
