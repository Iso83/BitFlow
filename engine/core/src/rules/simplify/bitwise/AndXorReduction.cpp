#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_And_Xor_Reduction(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = (*store)[b];
            if (exprB.op != OpType::Xor || exprB.inputs.size() != 2)
                continue;

            if (exprB.inputs[0] == a || exprB.inputs[1] == a)
                return true;
        }
    }

    return false;
}

static ExprId Rewrite_And_Xor_Reduction(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& x = (*store)[e.inputs[i]];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[e.inputs[j]];
            if (other.op != OpType::Xor || other.inputs.size() != 2)
                continue;

            ExprId y;
            if (other.inputs[0] == e.inputs[i])
                y = other.inputs[1];
            else if (other.inputs[1] == e.inputs[i])
                y = other.inputs[0];
            else
                continue;

            std::vector<ExprId> newInputs;
            newInputs.reserve(e.inputs.size());

            for (size_t k = 0; k < e.inputs.size(); ++k) {
                if (k == j)
                    continue;

                if (k == i) {
                    newInputs.push_back(e.inputs[i]);
                    newInputs.push_back(store->create(OpType::Not, {y}, e.bitWidth).id);
                    continue;
                }

                newInputs.push_back(e.inputs[k]);
            }

            return store->create(OpType::And, std::move(newInputs), e.bitWidth).id;
        }
    }

    _ASSERT(false);
    return id;
}

Rule Get_And_Xor_Reduction_Rule() {
    return Rule{And_Xor_Reduction,
                &Match_And_Xor_Reduction,
                &Rewrite_And_Xor_Reduction,
                {Normalize::Flatten, Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
