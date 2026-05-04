#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_And_Reduction(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Xor)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const ExprId x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = store->get(e.inputs[j]);
            if (other.op != OpType::And)
                continue;

            for (auto arg : other.inputs) {
                if (arg == x)
                    return true;
            }
        }
    }

    return false;
}

static ExprId Rewrite_Xor_And_Reduction(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        ExprId x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = store->get(e.inputs[j]);
            if (other.op != OpType::And)
                continue;

            std::vector<ExprId> andRemainder;
            andRemainder.reserve(other.inputs.size());

            bool removedX = false;
            for (auto in : other.inputs) {
                if (!removedX && in == x) {
                    removedX = true;
                    continue;
                }

                andRemainder.push_back(in);
            }

            if (!removedX)
                continue;

            ExprId yExpr;
            if (andRemainder.empty())
                yExpr = store->makeTrue(e.bitWidth).id;
            else if (andRemainder.size() == 1)
                yExpr = andRemainder[0];
            else
                yExpr = store->create(OpType::And, std::move(andRemainder), e.bitWidth).id;

            ExprId replacement =
                store->create(OpType::And, {x, store->create(OpType::Not, {yExpr}, e.bitWidth).id}, e.bitWidth).id;

            std::vector<ExprId> newInputs;
            newInputs.reserve(e.inputs.size() - 1);

            for (size_t k = 0; k < e.inputs.size(); ++k) {
                if (k == i) {
                    newInputs.push_back(replacement);
                    continue;
                }

                if (k == j)
                    continue;

                newInputs.push_back(e.inputs[k]);
            }

            if (newInputs.empty())
                return store->makeFalse(e.bitWidth).id;

            if (newInputs.size() == 1)
                return newInputs[0];

            return store->create(OpType::Xor, std::move(newInputs), e.bitWidth).id;
        }
    }

    _ASSERT(false);
    return id;
}

Rule Get_Xor_And_Reduction_Rule() {
    return Rule{RuleId::Simplify_XorAndReduction,
                &Match_Xor_And_Reduction,
                &Rewrite_Xor_And_Reduction,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order, RuleId::Simplify_AndXorReduction},
                RuleFlags::None,
                "Simplify_XorAndReduction"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
