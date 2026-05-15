#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_XorNotReduction(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr& a = (*store)[e.inputs[i]];
        if (a.op != OpType::Not || a.inputs.size() != 1)
            continue;

        const ExprId x = a.inputs[0];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[e.inputs[j]];
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

static ExprId Rewrite_XorNotReduction(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const std::vector<ExprId> inputs = e.inputs;
    const Types::BitWidth bitWidth = e.bitWidth;

    for (size_t i = 0; i < inputs.size(); ++i) {
        const ExprId notX = inputs[i];
        const Expr& exprNotX = (*store)[notX];
        if (exprNotX.op != OpType::Not || exprNotX.inputs.size() != 1)
            continue;

        ExprId x = exprNotX.inputs[0];

        for (size_t j = 0; j < inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr& other = (*store)[inputs[j]];
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
                yExpr = store->makeFalse(bitWidth).id;
            else if (xorRemainder.size() == 1)
                yExpr = xorRemainder[0];
            else
                yExpr = store->create(OpType::Xor, std::move(xorRemainder), bitWidth).id;

            std::vector<ExprId> newInputs;
            newInputs.reserve(inputs.size() + 1);

            for (size_t k = 0; k < inputs.size(); ++k) {
                if (k == i) {
                    newInputs.push_back(yExpr);
                    newInputs.push_back(notX);
                    continue;
                }

                if (k == j)
                    continue;

                newInputs.push_back(inputs[k]);
            }

            if (newInputs.empty())
                return store->makeTrue(bitWidth).id;

            if (newInputs.size() == 1)
                return newInputs[0];

            return store->create(OpType::And, std::move(newInputs), bitWidth).id;
        }
    }

    _ASSERT(false);
    return id;
}

Rule Get_XorNotReduction_Rule() {
    return Rule{XorNotReduction, &Match_XorNotReduction, &Rewrite_XorNotReduction, {AndXorReduction}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
