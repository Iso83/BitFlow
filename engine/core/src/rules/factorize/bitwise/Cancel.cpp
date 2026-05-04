#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Expression;

static bool ContainsId(const Expr& e, uint32_t id) {
    for (const Expr* in : e.inputs) {
        if (in->id.value() == id)
            return true;
    }

    return false;
}

static Expr* BuildXorResidual(const std::vector<Expr*>& terms) {
    if (terms.empty())
        return Expression::ConstPool::Get(0);

    if (terms.size() == 1)
        return terms[0];

    Expr* n = new Expr{};
    n->op = OpType::Xor;
    n->inputs = terms;
    return n;
}

#pragma region Match
static bool Match_Xor_Xor_CancelPair(const Expr& e) {
    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> childCounts;

    for (const Expr* in : e.inputs) {
        if (in->op != OpType::Xor || in->inputs.size() < 2)
            continue;

        std::unordered_map<uint32_t, bool> seenInChild;
        seenInChild.reserve(in->inputs.size());

        for (const Expr* term : in->inputs) {
            const uint32_t key = term->id.value();

            if (seenInChild[key])
                continue;

            seenInChild[key] = true;
            childCounts[key]++;

            if (childCounts[key] >= 2)
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Xor_Xor_CancelPair(Expr& e) {
    std::unordered_map<uint32_t, int> childCounts;
    std::unordered_map<uint32_t, Expr*> firstSeen;

    for (Expr* in : e.inputs) {
        if (in->op != OpType::Xor || in->inputs.size() < 2)
            continue;

        std::unordered_map<uint32_t, bool> seenInChild;
        seenInChild.reserve(in->inputs.size());

        for (Expr* term : in->inputs) {
            const uint32_t key = term->id.value();

            if (seenInChild[key])
                continue;

            seenInChild[key] = true;
            childCounts[key]++;

            if (!firstSeen.count(key))
                firstSeen[key] = term;
        }
    }

    uint32_t commonId = 0;
    Expr* common = nullptr;

    for (Expr* in : e.inputs) {
        if (in->op != OpType::Xor || in->inputs.size() < 2)
            continue;

        for (Expr* term : in->inputs) {
            const uint32_t key = term->id.value();

            if (childCounts[key] >= 2) {
                commonId = key;
                common = firstSeen[key];
                break;
            }
        }

        if (common != nullptr)
            break;
    }

    if (common == nullptr)
        return nullptr;

    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    int matchedChildren = 0;

    for (Expr* in : e.inputs) {
        if (in->op == OpType::Xor && in->inputs.size() >= 2 && ContainsId(*in, commonId)) {
            matchedChildren++;

            std::vector<Expr*> residual;
            residual.reserve(in->inputs.size());

            for (Expr* term : in->inputs) {
                if (term->id.value() != commonId)
                    residual.push_back(term);
            }

            Expr* reduced = BuildXorResidual(residual);

            if (!(reduced->op == OpType::Const && reduced->constValue == 0))
                newInputs.push_back(reduced);
        } else
            newInputs.push_back(in);
    }

    if ((matchedChildren & 1) != 0)
        newInputs.push_back(common);

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    auto* n = Expression::MakeOpInterned(OpType::Xor, std::move(newInputs));
    return n;
}
#pragma endregion

Rule Get_Xor_Pair_Cancel_Rule() {
    return Rule{RuleId::Factorize_XorPairCancel,
                &Match_Xor_Xor_CancelPair,
                &Rewrite_Xor_Xor_CancelPair,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Simplify_XorCancel},
                RuleFlags::Factorizing,
                "Factorize_XorPairCancel"};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise
