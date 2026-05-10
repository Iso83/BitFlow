#include "expression/ExprUtils.h"

#include <BitFlow/core/helper/Attributes.h>
#include <BitFlow/core/rules/Rule.h>
#include <stdexcept>
#include <vector>

namespace BitFlow::Core::Rules::Normalize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool IsConstFalse(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Const && e.inputs.empty() && store->isFalse(id);
}

static bool Match_Rotate_ModuloBitWidth(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::RotL && e.op != OpType::RotR)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& amount = (*store)[e.inputs[1]];
    if (!(amount.op == OpType::Const && amount.inputs.empty()))
        return false;

    return e.bitWidth > 0 && amount.knownValue >= e.bitWidth;
}

static ExprId Rewrite_Rotate_ModuloBitWidth(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    const Expr& amount = (*store)[e.inputs[1]];

    const Types::ExprChunk reduced = amount.knownValue % e.bitWidth;

    OpType op = e.op;
    Types::BitWidth bw = e.bitWidth;

    return store->create(op, {e.inputs[0], store->createConstant(reduced, bw).id}, bw).id;
}

Rule Get_Rotate_ModuloBitWidth_Rule() {
    return Rule{Rotate_ModuloBitWidth, &Match_Rotate_ModuloBitWidth, &Rewrite_Rotate_ModuloBitWidth};
}

} // namespace BitFlow::Core::Rules::Normalize::Bitwise