#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;
using OpType = AST::OpType;
using namespace BitFlow::Core::Expression;

static bool Match_Xor_And_Reduction(const Expr& e) {
    if (e.op != OpType::Xor)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr* x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr* other = e.inputs[j];
            if (other->op != OpType::And)
                continue;

            for (const Expr* arg : other->inputs) {
                if (arg->id.value() == x->id.value())
                    return true;
            }
        }
    }

    return false;
}

static Expr* Rewrite_Xor_And_Reduction(Expr& e) {
    for (size_t i = 0; i < e.inputs.size(); ++i) {
        Expr* x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            Expr* other = e.inputs[j];
            if (other->op != OpType::And)
                continue;

            std::vector<Expr*> andRemainder;
            andRemainder.reserve(other->inputs.size());

            bool removedX = false;
            for (Expr* in : other->inputs) {
                if (!removedX && in->id.value() == x->id.value()) {
                    removedX = true;
                    continue;
                }

                andRemainder.push_back(in);
            }

            if (!removedX)
                continue;

            Expr* yExpr = nullptr;
            if (andRemainder.empty())
                yExpr = ConstPool::Get(1);
            else if (andRemainder.size() == 1)
                yExpr = andRemainder[0];
            else
                yExpr = MakeOpInterned(OpType::And, std::move(andRemainder));

            Expr* replacement = MakeOpInterned(OpType::And, {x, MakeOpInterned(OpType::Not, {yExpr})});

            std::vector<Expr*> newInputs;
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
                return ConstPool::Get(0);

            if (newInputs.size() == 1)
                return newInputs[0];

            return MakeOpInterned(OpType::Xor, std::move(newInputs));
        }
    }

    return nullptr;
}

Rule Get_Xor_And_Reduction_Rule() {
    return Rule{RuleId::Simplify_XorAndReduction,
                &Match_Xor_And_Reduction,
                &Rewrite_Xor_And_Reduction,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order, RuleId::Simplify_AndXorReduction}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
