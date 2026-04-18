#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using Expr = AST::Expr;
using OpType = AST::OpType;
using namespace BitFlow::Core::Expression;

static size_t CountExprTreeNodes(const Expr* e) {
    size_t nodes = 1;
    for (const Expr* in : e->inputs)
        nodes += CountExprTreeNodes(in);
    return nodes;
}

static bool Match_Add_CommonFactor(const Expr& e) {
    if (e.op != OpType::Add || e.inputs.size() != 2)
        return false;

    const Expr* lhs = e.inputs[0];
    const Expr* rhs = e.inputs[1];
    return lhs->op == OpType::Mul && rhs->op == OpType::Mul && lhs->inputs.size() == 2 && rhs->inputs.size() == 2;
}

static Expr* Rewrite_Add_CommonFactor(Expr& e) {
    Expr* lhs = e.inputs[0];
    Expr* rhs = e.inputs[1];

    Expr* common = nullptr;
    Expr* lhsOther = nullptr;
    Expr* rhsOther = nullptr;

    for (Expr* l : lhs->inputs) {
        for (Expr* r : rhs->inputs) {
            if (l->id == r->id) {
                common = l;
                lhsOther = (lhs->inputs[0] == l) ? lhs->inputs[1] : lhs->inputs[0];
                rhsOther = (rhs->inputs[0] == r) ? rhs->inputs[1] : rhs->inputs[0];
                break;
            }
        }
        if (common)
            break;
    }

    if (!common || !lhsOther || !rhsOther)
        return nullptr;

    Expr* innerAdd = MakeOpInterned(OpType::Add, {lhsOther, rhsOther});
    Expr* candidate = MakeOpInterned(OpType::Mul, {common, innerAdd});

    if (CountExprTreeNodes(candidate) > CountExprTreeNodes(&e))
        return nullptr;

    return candidate;
}

Rule Get_Add_CommonFactor_Rule() {
    return Rule{RuleId::Factorize_AddCommonFactor,
                &Match_Add_CommonFactor,
                &Rewrite_Add_CommonFactor,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
