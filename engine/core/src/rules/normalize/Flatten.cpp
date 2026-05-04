#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Flatten(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.empty())
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (e.op == exprIn.op)
            return true;
    }

    return false;
}

static ExprId Rewrite_Flatten(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (!Expression::IsCommutative(e.op)) {
        _ASSERT(false);
        return id;
    }

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (e.op == exprIn.op) {
            for (auto sub : exprIn.inputs)
                newInputs.push_back(sub);
        } else
            newInputs.push_back(in);
    }

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

Rule Get_Flatten_Rule() {
    return Rule{RuleId::Normalize_Flatten, &Match_Flatten,     &Rewrite_Flatten, Stage_Normalize, {},
                RuleFlags::None,           "Normalize_Flatten"};
}

} // namespace BitFlow::Core::Rules::Normalize
