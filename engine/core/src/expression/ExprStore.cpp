#include "expression/ExprIntern.h"

#include <BitFlow/core/expression/ExprStore.h>

namespace BitFlow::Core::Expression {
ExprOld* MakeOpInterned(OpType op, std::vector<ExprOld*> inputs) {
    auto* e = new ExprOld{};
    e->op = op;
    e->inputs = std::move(inputs);
    return ExprIntern::Intern(e);
}
} // namespace BitFlow::Core::Expression