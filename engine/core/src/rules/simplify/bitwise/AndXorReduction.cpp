#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;
using OpType = AST::OpType;
using namespace BitFlow::Core::Expression;

static bool Match_And_Xor_Reduction(const Expr& e) {
    if (e.op != OpType::And || e.inputs.size() != 2)
        return false;

    const Expr* a = e.inputs[0];
    const Expr* b = e.inputs[1];

    if (b->op == OpType::Xor && b->inputs.size() == 2) {
        if (b->inputs[0]->id.value() == a->id.value() || b->inputs[1]->id.value() == a->id.value())
            return true;
    }

    if (a->op == OpType::Xor && a->inputs.size() == 2) {
        if (a->inputs[0]->id.value() == b->id.value() || a->inputs[1]->id.value() == b->id.value())
            return true;
    }

    return false;
}

static Expr* Rewrite_And_Xor_Reduction(Expr& e) {
    Expr* x = nullptr;
    Expr* y = nullptr;

    Expr* a = e.inputs[0];
    Expr* b = e.inputs[1];

    if (b->op == OpType::Xor && b->inputs.size() == 2) {
        if (b->inputs[0]->id.value() == a->id.value()) {
            x = a;
            y = b->inputs[1];
        } else if (b->inputs[1]->id.value() == a->id.value()) {
            x = a;
            y = b->inputs[0];
        }
    } else if (a->op == OpType::Xor && a->inputs.size() == 2) {
        if (a->inputs[0]->id.value() == b->id.value()) {
            x = b;
            y = a->inputs[1];
        } else if (a->inputs[1]->id.value() == b->id.value()) {
            x = b;
            y = a->inputs[0];
        }
    }

    if (!x || !y)
        return nullptr;

    return MakeOpInterned(OpType::And, {x, MakeOpInterned(OpType::Not, {y})});
}

Rule Get_And_Xor_Reduction_Rule() {
    return Rule{RuleId::Simplify_AndXorReduction,
                &Match_And_Xor_Reduction,
                &Rewrite_And_Xor_Reduction,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
