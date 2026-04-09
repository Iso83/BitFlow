#include "rules/RuleCommon.h"

#include <BitFlow/core/Rule.h>
#include <vector>

namespace BitFlow::Core {

static Expr* Rewrite_Remove_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!(in->isConst && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return e.inputs[0];

    if (newInputs.size() == 1)
        return newInputs[0];

    e.inputs = std::move(newInputs);
    return &e;
}

Rule Get_Add_Zero_Rule() {
    return Rule{&Match_Zero<OpType::Add>, &Rewrite_Remove_Zero};
}

Rule Get_Xor_Zero_Rule() {
    return Rule{&Match_Zero<OpType::Xor>, &Rewrite_Remove_Zero};
}

} // namespace BitFlow::Core