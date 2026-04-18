#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using Expr = AST::Expr;
using OpType = AST::OpType;
using ConstPool = Expression::ConstPool;
using namespace BitFlow::Core::Expression;

static size_t CountExprTreeNodes(const Expr* e) {
    size_t nodes = 1;
    for (const Expr* in : e->inputs)
        nodes += CountExprTreeNodes(in);
    return nodes;
}

static bool Match_Add_CommonFactor(const Expr& e) {
    if (e.op != OpType::Add || e.inputs.size() < 2)
        return false;

    // identical addends: t + t (+ ...)
    std::unordered_map<const Expr*, int> counts;
    for (const Expr* input : e.inputs) {
        if (++counts[input] >= 2)
            return true;
    }

    // common multiplicative factor: x*y + x*z
    for (size_t i = 0; i < e.inputs.size(); ++i) {
        const Expr* lhs = e.inputs[i];
        if (lhs->op != OpType::Mul || lhs->inputs.size() != 2)
            continue;

        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            const Expr* rhs = e.inputs[j];
            if (rhs->op != OpType::Mul || rhs->inputs.size() != 2)
                continue;

            for (const Expr* l : lhs->inputs) {
                for (const Expr* r : rhs->inputs) {
                    if (l->id == r->id)
                        return true;
                }
            }
        }
    }

    return false;
}

static Expr* Rewrite_Add_CommonFactor(Expr& e) {
    std::unordered_map<Expr*, int> counts;
    std::vector<Expr*> order;
    order.reserve(e.inputs.size());

    // collapse identical addends into k*term
    for (Expr* input : e.inputs) {
        if (counts.emplace(input, 0).second)
            order.push_back(input);
        counts[input]++;
    }

    std::vector<Expr*> normalizedAddTerms;
    normalizedAddTerms.reserve(order.size());
    for (Expr* term : order) {
        const int count = counts[term];
        if (count == 1) {
            normalizedAddTerms.push_back(term);
            continue;
        }

        normalizedAddTerms.push_back(MakeOpInterned(OpType::Mul, {ConstPool::Get(static_cast<uint64_t>(count)), term}));
    }

    if (normalizedAddTerms.empty())
        return nullptr;
    if (normalizedAddTerms.size() == 1)
        return normalizedAddTerms[0];

    // try common-factor extraction across binary multiplications
    std::unordered_map<Expr*, int> factorFrequency;
    for (Expr* term : normalizedAddTerms) {
        if (term->op != OpType::Mul || term->inputs.size() != 2)
            continue;
        factorFrequency[term->inputs[0]]++;
        factorFrequency[term->inputs[1]]++;
    }

    Expr* common = nullptr;
    int bestFrequency = 0;
    for (Expr* term : normalizedAddTerms) {
        if (term->op != OpType::Mul || term->inputs.size() != 2)
            continue;
        for (Expr* factor : term->inputs) {
            const int frequency = factorFrequency[factor];
            if (frequency > bestFrequency) {
                bestFrequency = frequency;
                common = factor;
            }
        }
    }

    if (!common || bestFrequency < 2) {
        Expr* candidate = MakeOpInterned(OpType::Add, normalizedAddTerms);
        if (CountExprTreeNodes(candidate) > CountExprTreeNodes(&e))
            return nullptr;
        return candidate;
    }

    std::vector<Expr*> sharedInnerTerms;
    std::vector<Expr*> untouchedTerms;
    sharedInnerTerms.reserve(normalizedAddTerms.size());
    untouchedTerms.reserve(normalizedAddTerms.size());

    for (Expr* term : normalizedAddTerms) {
        if (term->op == OpType::Mul && term->inputs.size() == 2) {
            if (term->inputs[0]->id == common->id) {
                sharedInnerTerms.push_back(term->inputs[1]);
                continue;
            }
            if (term->inputs[1]->id == common->id) {
                sharedInnerTerms.push_back(term->inputs[0]);
                continue;
            }
        }
        untouchedTerms.push_back(term);
    }

    if (sharedInnerTerms.size() < 2) {
        Expr* candidate = MakeOpInterned(OpType::Add, normalizedAddTerms);
        if (CountExprTreeNodes(candidate) > CountExprTreeNodes(&e))
            return nullptr;
        return candidate;
    }

    Expr* innerAdd = MakeOpInterned(OpType::Add, sharedInnerTerms);
    Expr* factored = MakeOpInterned(OpType::Mul, {common, innerAdd});

    std::vector<Expr*> finalAddTerms;
    finalAddTerms.reserve(untouchedTerms.size() + 1);
    for (Expr* term : untouchedTerms)
        finalAddTerms.push_back(term);
    finalAddTerms.push_back(factored);

    Expr* candidate = (finalAddTerms.size() == 1) ? finalAddTerms[0] : MakeOpInterned(OpType::Add, finalAddTerms);

    if (CountExprTreeNodes(candidate) > CountExprTreeNodes(&e))
        return nullptr;

    return candidate;
}

Rule Get_Add_CommonFactor_Rule() {
    return Rule{RuleId::Factorize_AddCommonFactor,
                &Match_Add_CommonFactor,
                &Rewrite_Add_CommonFactor,
                Stage_Factorize,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
