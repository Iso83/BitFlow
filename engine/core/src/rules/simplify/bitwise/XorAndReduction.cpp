#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_And_Reduction(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Xor)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const ExprId x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[e.inputs[j]];
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
    const Expr& e = (*store)[id];

    const auto inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    for (size_t i = 0; i < inputs.size(); ++i) {
        ExprId x = inputs[i];

        for (size_t j = 0; j < inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[inputs[j]];
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
                yExpr = store->makeTrue(bitWidth).id;
            else if (andRemainder.size() == 1)
                yExpr = andRemainder[0];
            else
                yExpr = store->create(OpType::And, std::move(andRemainder), bitWidth).id;

            ExprId notY = store->create(OpType::Not, {yExpr}, bitWidth).id;

            ExprId replacement = store->create(OpType::And, {x, notY}, bitWidth).id;

            std::vector<ExprId> newInputs;
            newInputs.reserve(inputs.size() - 1);

            for (size_t k = 0; k < inputs.size(); ++k) {
                if (k == i) {
                    newInputs.push_back(replacement);
                    continue;
                }

                if (k == j)
                    continue;

                newInputs.push_back(inputs[k]);
            }

            if (newInputs.empty())
                return store->makeFalse(bitWidth).id;

            if (newInputs.size() == 1)
                return newInputs[0];

            return store->create(OpType::Xor, std::move(newInputs), bitWidth).id;
        }
    }

    _ASSERT(false);
    return id;
}

Rule Get_Xor_And_Reduction_Rule() {
    return Rule{Xor_And_Reduction,
                &Match_Xor_And_Reduction,
                &Rewrite_Xor_And_Reduction,
                {Normalize::Flatten, Normalize::Order, And_Xor_Reduction}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
