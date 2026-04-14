#include "expression/ExprKeyBuilders.h"

#include "rules/RuleCommon.h"

#include <algorithm>

namespace BitFlow::Core::Expression {

using namespace BitFlow::Core::AST;
using namespace BitFlow::Core::Rules;

Key BuildCommutativeKey(const Expr* e) {
    Key k{};
    k.op = e->op;
    k.constValue = e->constValue;

    k.inputs.reserve(e->inputs.size());

    for (const Expr* in : e->inputs)
        k.inputs.push_back(in->id.value());

    if (IsCommutative(k.op))
        std::sort(k.inputs.begin(), k.inputs.end());

    if (e->inputs.empty())
        k.inputs.push_back(e->id.value());

    return k;
}

} // namespace BitFlow::Core::Expression
