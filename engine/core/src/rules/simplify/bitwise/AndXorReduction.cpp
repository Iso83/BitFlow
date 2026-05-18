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
            if (exprB.op != OpType::Xor)
                continue;

            for (auto xorIn : exprB.inputs) {
                if (xorIn == a)
                    return true;
            }
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
            if (other.op != OpType::Xor || other.inputs.size() < 2)
                continue;

            for (size_t xi = 0; xi < other.inputs.size(); ++xi) {
                if (other.inputs[xi] != x)
                    continue;

                std::vector<ExprId> xorRemainder;
                xorRemainder.reserve(other.inputs.size() - 1);

                for (size_t ri = 0; ri < other.inputs.size(); ++ri) {
                    if (ri != xi)
                        xorRemainder.push_back(other.inputs[ri]);
                }

                const ExprId y = xorRemainder.size() == 1
                                     ? xorRemainder[0]
                                     : store->create(OpType::Xor, std::move(xorRemainder), bitWidth).id;
                const ExprId notY = store->create(OpType::Not, {y}, bitWidth).id;

                std::vector<ExprId> newInputs;
                newInputs.reserve(inputs.size());

                for (size_t k = 0; k < inputs.size(); ++k) {
                    if (k == j)
                        continue;

                    if (k == i) {
                        newInputs.push_back(x);
                        newInputs.push_back(notY);
                    } else {
                        newInputs.push_back(inputs[k]);
                    }
                }

                return store->create(OpType::And, std::move(newInputs), bitWidth).id;
            }
        }
    }

    BF_CORE_ASSERT(false);
    return id;
}

Rule Get_AndXorReduction_Rule() {
    return Rule{AndXorReduction, &Match_AndXorReduction, &Rewrite_AndXorReduction, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
