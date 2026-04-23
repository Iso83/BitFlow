#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;
using OpType = AST::OpType;
using namespace BitFlow::Core::Expression;

static bool Match_And_Xor_Reduction(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr* x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            const Expr* other = e.inputs[j];
            if (other->op != OpType::Xor || other->inputs.size() != 2)
                continue;

            if (other->inputs[0]->id.value() == x->id.value() || other->inputs[1]->id.value() == x->id.value())
                return true;
        }
    }

    return false;
}

static Expr* Rewrite_And_Xor_Reduction(Expr& e) {
    for (size_t i = 0; i < e.inputs.size(); ++i) {
        Expr* x = e.inputs[i];

        for (size_t j = 0; j < e.inputs.size(); ++j) {
            if (i == j)
                continue;

            Expr* other = e.inputs[j];
            if (other->op != OpType::Xor || other->inputs.size() != 2)
                continue;

            Expr* y = nullptr;
            if (other->inputs[0]->id.value() == x->id.value())
                y = other->inputs[1];
            else if (other->inputs[1]->id.value() == x->id.value())
                y = other->inputs[0];

            if (!y)
                continue;

            std::vector<Expr*> newInputs;
            newInputs.reserve(e.inputs.size());

            for (size_t k = 0; k < e.inputs.size(); ++k) {
                if (k == j)
                    continue;

                if (k == i) {
                    newInputs.push_back(x);
                    newInputs.push_back(MakeOpInterned(OpType::Not, {y}));
                    continue;
                }

                newInputs.push_back(e.inputs[k]);
            }

            return MakeOpInterned(OpType::And, std::move(newInputs));
        }
    }

    return nullptr;
}

Rule Get_And_Xor_Reduction_Rule() {
    return Rule{RuleId::Simplify_AndXorReduction, &Match_And_Xor_Reduction, &Rewrite_And_Xor_Reduction, Stage_Simplify, {RuleId::Normalize_Flatten, RuleId::Normalize_Order}, RuleFlags::None, "Simplify_AndXorReduction"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
