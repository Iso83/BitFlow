#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_AddFold(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Add)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const)
            constCount++;
    }

    return constCount >= 2;
}

static ExprId Rewrite_AddFold(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    Types::ExprChunk acc = 0;
    bool hasConst = false;

    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    const Types::ExprChunk mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            hasConst = true;
        } else {
            nonConst.push_back(inId);
        }
    }

    if (!hasConst)
        return id;

    const Types::BitWidth bitWidth = e.bitWidth;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Add, std::move(nonConst), bitWidth).id;
}

Rule Get_AddFold_Rule() {
    return Rule{AddFold, &Match_AddFold, &Rewrite_AddFold, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
