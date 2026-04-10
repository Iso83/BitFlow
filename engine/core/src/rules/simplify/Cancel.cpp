#include "expression/ExprClone.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

#pragma region Match
static bool Match_And_Cancel(const Expr& e) {
    if (e.op != AST::OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i - 1]->id == e.inputs[i]->id)
            return true;
    }

    return false;
}

static bool Match_Or_Cancel(const Expr& e) {
    if (e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i - 1]->id == e.inputs[i]->id)
            return true;
    }

    return false;
}

static bool Match_Xor_Cancel(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            if (e.inputs[i]->id == e.inputs[j]->id)
                return true;
        }
    }

    return false;
}

static bool Match_Xor_DuplicateCancel(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;

    for (const Expr* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Cancel(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    Expr* prev = nullptr;

    for (Expr* in : e.inputs) {
        if (prev != nullptr && prev->id == in->id)
            continue;

        newInputs.push_back(in);
        prev = in;
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(1);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Or_Cancel(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    Expr* prev = nullptr;

    for (Expr* in : e.inputs) {
        if (prev != nullptr && prev->id == in->id)
            continue;

        newInputs.push_back(in);
        prev = in;
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Xor_Cancel(Expr& e) {
    std::vector<Expr*> result;

    for (Expr* in : e.inputs) {
        bool found = false;

        for (auto it = result.begin(); it != result.end(); ++it) {
            if ((*it)->id == in->id) {
                result.erase(it); // cancel pair
                found = true;
                break;
            }
        }

        if (!found)
            result.push_back(in);
    }

    if (result.empty())
        return Expression::ConstPool::Get(0);

    if (result.size() == 1)
        return result[0];

    e.inputs = std::move(result);
    return &e;
}

static Expr* Rewrite_Xor_DuplicateCancel(Expr& e) {
    std::unordered_map<uint32_t, int> counts;

    for (Expr* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;
    }

    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    std::unordered_map<uint32_t, int> emitted;

    for (Expr* in : e.inputs) {
        const uint32_t key = in->id.value();

        if ((counts[key] % 2) == 1 && emitted[key] == 0) {
            newInputs.push_back(in);
            emitted[key] = 1;
        }
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);

    return target;
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
                &Match_Xor_Cancel,
                &Rewrite_Xor_Cancel,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Xor_DuplicateCancel_Rule() {
    return Rule{RuleId::Simplify_XorDuplicateCancel,
                &Match_Xor_DuplicateCancel,
                &Rewrite_Xor_DuplicateCancel,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify