#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_AndXorReduction(const ExprStore* store, ExprId id) {
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

static ExprId Rewrite_AndXorReduction(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const std::vector<ExprId> inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    for (size_t i = 0; i < inputs.size(); ++i) {
        const ExprId x = inputs[i];

        for (size_t j = 0; j < inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[inputs[j]];
            if (other.op != OpType::Xor || other.inputs.size() != 2)
                continue;

            ExprId y;

            if (other.inputs[0] == x)
                y = other.inputs[1];
            else if (other.inputs[1] == x)
                y = other.inputs[0];
            else
                continue;

            const ExprId notY = store->create(OpType::Not, {y}, bitWidth).id;

            std::vector<ExprId> newInputs;
            newInputs.reserve(inputs.size());

            for (size_t k = 0; k < inputs.size(); ++k) {
                if (k == j)
                    continue;

                if (k == i) {
                    newInputs.push_back(x);
                    newInputs.push_back(notY);
                    continue;
                }

                newInputs.push_back(inputs[k]);
            }

            return store->create(OpType::And, std::move(newInputs), bitWidth).id;
        }
    }

    BF_CORE_ASSERT(false);
    return id;
}

Rule Get_AndXorReduction_Rule() {
    return Rule{AndXorReduction, &Match_AndXorReduction, &Rewrite_AndXorReduction, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
