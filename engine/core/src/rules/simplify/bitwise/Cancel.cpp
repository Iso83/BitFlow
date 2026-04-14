#include "expression/ExprClone.h"
#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;
using OpType = AST::OpType;
using namespace BitFlow::Core::Expression;

static Expr* BuildXorParityResult(const std::vector<Expr*>& terms) {
    if (terms.empty())
        return ConstPool::Get(0);

    if (terms.size() == 1)
        return terms[0];

    return MakeOpInterned(OpType::Xor, terms);
}

#pragma region Match
static bool Match_And_Cancel(const Expr& e) {
    if (e.op != AST::OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;
    counts.reserve(e.inputs.size());

    for (const Expr* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}

static bool Match_Or_Cancel(const Expr& e) {
    if (e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;
    counts.reserve(e.inputs.size());

    for (const Expr* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}

static bool Match_XorCancel(const Expr& e) {
    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    uint32_t constParity = 0;

    for (const Expr* in : e.inputs) {
        if (in->isConst())
            constParity ^= in->constValue;
    }

    if (constParity != 0)
        return true;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (!e.inputs[i]->isConst() && !e.inputs[i - 1]->isConst() &&
            e.inputs[i - 1]->id.value() == e.inputs[i]->id.value())
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Cancel(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<uint32_t, bool> seen;
    seen.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        const uint32_t key = in->id.value();

        if (seen[key])
            continue;

        seen[key] = true;
        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ConstPool::Get(1);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Or_Cancel(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<uint32_t, bool> seen;
    seen.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        const uint32_t key = in->id.value();

        if (seen[key])
            continue;

        seen[key] = true;
        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_XorCancel(Expr& e) {
    std::vector<Expr*> oddTerms;
    oddTerms.reserve(e.inputs.size());

    uint32_t constParity = 0;

    size_t i = 0;
    while (i < e.inputs.size()) {
        Expr* cur = e.inputs[i];

        if (cur->isConst()) {
            constParity ^= cur->constValue;
            ++i;
            continue;
        }

        size_t j = i + 1;
        while (j < e.inputs.size() && !e.inputs[j]->isConst() && e.inputs[j]->id.value() == cur->id.value())
            ++j;

        const size_t count = j - i;
        if (count & 1u)
            oddTerms.push_back(cur);

        i = j;
    }

    if (constParity != 0)
        oddTerms.push_back(ConstPool::Get(constParity));

    std::sort(oddTerms.begin(), oddTerms.end(), [](Expr* a, Expr* b) { return a->id.value() < b->id.value(); });

    return BuildXorParityResult(oddTerms);
}
#pragma endregion

Rule Get_And_Cancel_Rule() {
    return Rule{RuleId::Simplify_AndCancel,
                &Match_And_Cancel,
                &Rewrite_And_Cancel,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Or_Cancel_Rule() {
    return Rule{
        RuleId::Simplify_OrCancel, &Match_Or_Cancel, &Rewrite_Or_Cancel, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

Rule Get_Xor_Cancel_Rule() {
    return Rule{RuleId::Simplify_XorCancel,
                &Match_XorCancel,
                &Rewrite_XorCancel,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
