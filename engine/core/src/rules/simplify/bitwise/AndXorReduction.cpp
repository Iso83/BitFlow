#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_And_Xor_Reduction(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = store->get(b);
            if (exprB.op != OpType::Xor || exprB.inputs.size() != 2)
                continue;

            if (exprB.inputs[0] == a || exprB.inputs[1] == a)
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_And_Xor_Reduction(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& x = store->get(e.inputs[i]);

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = store->get(e.inputs[j]);
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
                    newInputs.push_back(Make_Expr_Not(store, y).id);
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
#pragma endregion

Rule Get_And_Xor_Reduction_Rule() {
    return Rule{RuleId::Simplify_AndXorReduction,
                &Match_And_Xor_Reduction,
                &Rewrite_And_Xor_Reduction,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::None,
                "Simplify_AndXorReduction"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
