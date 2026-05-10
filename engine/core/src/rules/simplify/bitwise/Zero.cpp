#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static ExprId Rewrite_Remove_Zero(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
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

Rule Get_Xor_Zero_Rule() {
    return Rule{Xor_Zero, &Match_Zero<OpType::Xor>, &Rewrite_Remove_Zero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
