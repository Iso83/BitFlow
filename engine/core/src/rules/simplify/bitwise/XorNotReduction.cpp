#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_Not_Reduction(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& a = store->get(e.inputs[i]);
        if (a.op != OpType::Not || a.inputs.size() != 1)
            continue;

        const ExprId x = a.inputs[0];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = store->get(e.inputs[j]);
            if (other.op != OpType::Xor)
                continue;

            for (auto xorIn : other.inputs) {
                if (xorIn == x)
                    return true;
            }
        }
    }

    return false;
}

static ExprId Rewrite_Xor_Not_Reduction(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const ExprId notX = e.inputs[i];
        const Expr& exprNotX = store->get(notX);
        if (exprNotX.op != OpType::Not || exprNotX.inputs.size() != 1)
            continue;

        ExprId x = exprNotX.inputs[0];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = store->get(e.inputs[j]);
            if (other.op != OpType::Xor)
                continue;

            std::vector<ExprId> xorRemainder;
            xorRemainder.reserve(other.inputs.size());

            bool removedX = false;
            for (auto xorIn : other.inputs) {
                if (!removedX && xorIn == x) {
                    removedX = true;
                    continue;
                }

                xorRemainder.push_back(xorIn);
            }

            if (!removedX)
                continue;

            ExprId yExpr;
            if (xorRemainder.empty())
                yExpr = store->makeFalse(e.bitWidth).id;
            else if (xorRemainder.size() == 1)
                yExpr = xorRemainder[0];
            else
                yExpr = store->create(OpType::Xor, std::move(xorRemainder), e.bitWidth).id;

            std::vector<ExprId> newInputs;
            newInputs.reserve(e.inputs.size() + 1);

            for (size_t k = 0; k < e.inputs.size(); ++k) {
                if (k == i) {
                    newInputs.push_back(yExpr);
                    newInputs.push_back(notX);
                    continue;
                }

                if (k == j)
                    continue;

                newInputs.push_back(e.inputs[k]);
            }

            if (newInputs.empty())
                return store->makeTrue(e.bitWidth).id;

            if (newInputs.size() == 1)
                return newInputs[0];

            return store->create(OpType::And, std::move(newInputs), e.bitWidth).id;
        }
    }

    _ASSERT(false);
    return id;
}

Rule Get_Xor_Not_Reduction_Rule() {
    return Rule{RuleId::Simplify_XorNotReduction,
                &Match_Xor_Not_Reduction,
                &Rewrite_Xor_Not_Reduction,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order, RuleId::Simplify_AndXorReduction},
                RuleFlags::None,
                "Simplify_XorNotReduction"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
