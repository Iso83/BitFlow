#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Add_Fold(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != OpType::Add)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const)
            constCount++;
    }

    return constCount >= 2;
}

static ExprId Rewrite_Add_Fold(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    uint64_t acc = 0;
    bool hasConst = false;

    std::vector<ExprId> nonConst;
    nonConst.reserve(e.inputs.size());

    const uint64_t mask = Expr::fullMask(e.bitWidth);

    for (ExprId inId : e.inputs) {
        const Expr& in = store->get(inId);

        if (in.op == OpType::Const) {
            acc = (acc + in.knownValue) & mask;
            hasConst = true;
        } else {
            nonConst.push_back(inId);
        }
    }

    if (!hasConst)
        return id;

    if (acc != 0)
        nonConst.push_back(store->createConstant(acc, e.bitWidth).id);

    if (nonConst.empty())
        return store->createConstant(0, e.bitWidth).id;

    if (nonConst.size() == 1)
        return nonConst[0];

    return store->create(OpType::Add, std::move(nonConst), e.bitWidth).id;
}

Rule Get_Add_Fold_Rule() {
    return Rule{Add_Fold, &Match_Add_Fold, &Rewrite_Add_Fold, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
