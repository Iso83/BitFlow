#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = Expression::Expr;
using OpType = Expression::OpType;
using ConstPool = Expression::ConstPool;

static bool IsNotOf(Expr* a, Expr* b) {
    return a->op == OpType::Not && a->inputs.size() == 1 && a->inputs[0] == b;
}

#pragma region Match
static bool Match_Complement(const Expr& e) {
    if ((e.op != OpType::And && e.op != OpType::Or) || e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            Expr* a = e.inputs[i];
            Expr* b = e.inputs[j];

            if (IsNotOf(a, b) || IsNotOf(b, a))
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Complement(Expr& e) {
    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            Expr* a = e.inputs[i];
            Expr* b = e.inputs[j];

            if (IsNotOf(a, b) || IsNotOf(b, a)) {
                if (e.op == OpType::And)
                    return ConstPool::Get(0);

                if (e.op == OpType::Or)
                    return ConstPool::Get(1);
            }
        }
    }

    return nullptr;
}
#pragma endregion

Rule Get_Complement_Rule() {
    return Rule{RuleId::Simplify_Complement,
                &Match_Complement,
                &Rewrite_Complement,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Simplify_Idempotent},
                RuleFlags::None,
                "Simplify_Complement"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise