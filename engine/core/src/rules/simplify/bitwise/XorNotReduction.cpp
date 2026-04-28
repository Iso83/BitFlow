#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = Expression::ExprOld;
using OpType = Expression::OpType;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_Not_Reduction(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr* a = e.inputs[i];
        if (a->op != OpType::Not || a->inputs.size() != 1)
            continue;

        const Expr* x = a->inputs[0];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr* other = e.inputs[j];
            if (other->op != OpType::Xor)
                continue;

            for (const Expr* xorIn : other->inputs) {
                if (xorIn->id.value() == x->id.value())
                    return true;
            }
        }
    }

    return false;
}

static Expr* Rewrite_Xor_Not_Reduction(Expr& e) {
    for (size_t i = 0; i < e.inputs.size(); ++i) {
        Expr* notX = e.inputs[i];
        if (notX->op != OpType::Not || notX->inputs.size() != 1)
            continue;

        Expr* x = notX->inputs[0];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            Expr* other = e.inputs[j];
            if (other->op != OpType::Xor)
                continue;

            std::vector<Expr*> xorRemainder;
            xorRemainder.reserve(other->inputs.size());

            bool removedX = false;
            for (Expr* xorIn : other->inputs) {
                if (!removedX && xorIn->id.value() == x->id.value()) {
                    removedX = true;
                    continue;
                }

                xorRemainder.push_back(xorIn);
            }

            if (!removedX)
                continue;

            Expr* yExpr = nullptr;
            if (xorRemainder.empty())
                yExpr = ConstPool::Get(0);
            else if (xorRemainder.size() == 1)
                yExpr = xorRemainder[0];
            else
                yExpr = MakeOpInterned(OpType::Xor, std::move(xorRemainder));

            std::vector<Expr*> newInputs;
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
                return ConstPool::Get(1);

            if (newInputs.size() == 1)
                return newInputs[0];

            return MakeOpInterned(OpType::And, std::move(newInputs));
        }
    }

    return nullptr;
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
