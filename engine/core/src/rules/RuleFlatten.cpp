#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>

namespace BitFlow::Core {

static bool Match_Flatten(const Expr& e) {
    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (!in->isConst && !in->inputs.empty() && in->op == e.op)
            return true;
    }

    return false;
}

static Expr* Rewrite_Flatten(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!in->isConst && !in->inputs.empty() && in->op == e.op) {
            for (Expr* sub : in->inputs)
                newInputs.push_back(sub);
        } else {
            newInputs.push_back(in);
        }
    }

    e.inputs = std::move(newInputs);
    return &e;
}

Rule Get_Flatten_Rule() {
    return Rule{&Match_Flatten, &Rewrite_Flatten};
}

} // namespace BitFlow::Core