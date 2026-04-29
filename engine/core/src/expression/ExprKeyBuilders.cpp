#include "expression/ExprKeyBuilders.h"

#include <algorithm>

namespace BitFlow::Core::Expression {

Key BuildCommutativeKey(const ExprOld* e) {
    Key k{};
    k.op = e->op;
    k.constValue = e->constValue;

    k.inputs.reserve(e->inputs.size());

    for (const ExprOld* in : e->inputs)
        k.inputs.push_back(in->id.value());

    if (IsCommutative(k.op))
        std::sort(k.inputs.begin(), k.inputs.end());

    if (e->inputs.empty()) {
        if (e->op == OpType::Var)
            k.inputs.push_back(e->id.value());
        else if (e->op == OpType::Const)
            k.inputs.push_back(e->constValue);
    }

    return k;
}

} // namespace BitFlow::Core::Expression
