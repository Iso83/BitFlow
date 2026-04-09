#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

static bool Match_Xor_Cancel(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;

    for (const Expr* in : e.inputs)
        counts[in->id.value()]++;

    for (const auto& [_, count] : counts) {
        if (count > 1)
            return true;
    }

    return false;
}

static Expr* Rewrite_Xor_Cancel(Expr& e) {
    std::unordered_map<uint32_t, int> counts;
    std::unordered_map<uint32_t, Expr*> lookup;

    for (Expr* in : e.inputs) {
        auto id = in->id.value();
        counts[id]++;
        lookup[id] = in;
    }

    std::vector<Expr*> newInputs;

    for (const auto& [id, count] : counts) {
        if (count % 2 == 1)
            newInputs.push_back(lookup[id]);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* n = new Expr{};
    n->op = e.op;
    n->inputs = std::move(newInputs);
    return n;
}

Rule Get_Xor_Cancel_Rule() {
    return Rule{&Match_Xor_Cancel, &Rewrite_Xor_Cancel, Stage_Simplify};
}

} // namespace BitFlow::Core::Rules::Simplify