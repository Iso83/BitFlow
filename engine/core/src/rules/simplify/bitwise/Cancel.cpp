#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Expression;

static ExprOld* BuildXorParityResult(const std::vector<ExprOld*>& terms) {
    if (terms.empty())
        return ConstPool::Get(0);

    if (terms.size() == 1)
        return terms[0];

    return MakeOpInterned(OpType::Xor, terms);
}

#pragma region Match
static bool Match_And_Cancel(const ExprOld& e) {
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;
    counts.reserve(e.inputs.size());

    for (const ExprOld* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}

static bool Match_Or_Cancel(const ExprOld& e) {
    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;
    counts.reserve(e.inputs.size());

    for (const ExprOld* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}

static bool Match_XorCancel(const ExprOld& e) {
    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    uint32_t constParity = 0;

    for (const ExprOld* in : e.inputs) {
        if (in->op == OpType::Const)
            constParity ^= in->constValue;
    }

    if (constParity != 0)
        return true;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i]->op != OpType::Const && e.inputs[i - 1]->op != OpType::Const &&
            CompareExprCanonical(e.inputs[i - 1], e.inputs[i]) == 0)
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprOld* Rewrite_And_Cancel(ExprOld& e) {
    std::vector<ExprOld*> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<uint32_t, bool> seen;
    seen.reserve(e.inputs.size());

    for (ExprOld* in : e.inputs) {
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

    ExprOld* target = CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static ExprOld* Rewrite_Or_Cancel(ExprOld& e) {
    std::vector<ExprOld*> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<uint32_t, bool> seen;
    seen.reserve(e.inputs.size());

    for (ExprOld* in : e.inputs) {
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

    ExprOld* target = CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static ExprOld* Rewrite_XorCancel(ExprOld& e) {
    std::vector<ExprOld*> oddTerms;
    oddTerms.reserve(e.inputs.size());

    uint32_t constParity = 0;

    size_t i = 0;
    while (i < e.inputs.size()) {
        ExprOld* cur = e.inputs[i];

        if (cur->op == OpType::Const) {
            constParity ^= cur->constValue;
            ++i;
            continue;
        }

        size_t j = i + 1;
        while (j < e.inputs.size() && e.inputs[j]->op != OpType::Const && CompareExprCanonical(e.inputs[j], cur) == 0)
            ++j;

        const size_t count = j - i;
        if (count & 1u)
            oddTerms.push_back(cur);

        i = j;
    }

    if (constParity != 0)
        oddTerms.push_back(ConstPool::Get(constParity));

    std::sort(oddTerms.begin(), oddTerms.end(), [](ExprOld* a, ExprOld* b) { return CanonicalExprLess(a, b); });

    return BuildXorParityResult(oddTerms);
}
#pragma endregion

Rule Get_And_Cancel_Rule() {
    return Rule{RuleId::Simplify_AndCancel,  &Match_And_Cancel, &Rewrite_And_Cancel, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None,   "Simplify_AndCancel"};
}

Rule Get_Or_Cancel_Rule() {
    return Rule{RuleId::Simplify_OrCancel,   &Match_Or_Cancel, &Rewrite_Or_Cancel, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None,  "Simplify_OrCancel"};
}

Rule Get_Xor_Cancel_Rule() {
    return Rule{RuleId::Simplify_XorCancel,
                &Match_XorCancel,
                &Rewrite_XorCancel,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order},
                RuleFlags::None,
                "Simplify_XorCancel"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
