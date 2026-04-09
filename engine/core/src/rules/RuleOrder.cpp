#include "rules/RuleCommon.h"

#include <BitFlow/core/Rule.h>
#include <algorithm>

namespace BitFlow::Core {

// Canonical form

static bool Match_Order(const Expr& e) {
    if (!IsCommutative(e.op))
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i - 1]->id.value() > e.inputs[i]->id.value())
            return true;
    }

    return false;
}

static Expr* Rewrite_Order(Expr& e) {
    std::sort(e.inputs.begin(), e.inputs.end(), [](Expr* a, Expr* b) { return a->id.value() < b->id.value(); });

    return &e;
}

Rule Get_Order_Rule() {
    return Rule{&Match_Order, &Rewrite_Order};
}

} // namespace BitFlow::Core