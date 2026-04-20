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

    // linear multiplicity by base: x + x, x + x*k, x*m + x*n
    std::unordered_map<uint32_t, int> baseTermCounts;
    for (const Expr* input : e.inputs) {
        const Expr* base = input;
        if (input->op == OpType::Mul) {
            const Expr* nonConst = nullptr;
            bool hasConst = false;
            for (const Expr* factor : input->inputs) {
                if (factor->isConst()) {
                    hasConst = true;
                    continue;
                }
                if (nonConst != nullptr) {
                    nonConst = nullptr;
                    break;
                }
                nonConst = factor;
            }
            if (hasConst && nonConst != nullptr)
                base = nonConst;
        }

        if (++baseTermCounts[base->id.value()] >= 2)
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
    struct LinearTerm {
        Expr* base = nullptr;
        uint32_t coeff = 0;
    };

    auto DecomposeLinearTerm = [](Expr* term, LinearTerm& out) {
        if (term->op != OpType::Mul) {
            out.base = term;
            out.coeff = 1u;
            return true;
        }

        uint32_t coeff = 1u;
        Expr* base = nullptr;
        for (Expr* factor : term->inputs) {
            if (factor->isConst()) {
                coeff *= factor->constValue;
                continue;
            }

            if (base != nullptr)
                return false;
            base = factor;
        }

        if (base == nullptr)
            return false;

        out.base = base;
        out.coeff = coeff;
        return true;
    };

    std::unordered_map<uint32_t, uint32_t> coeffByBaseId;
    std::unordered_map<uint32_t, Expr*> baseExprById;
    std::vector<uint32_t> baseOrder;
    baseOrder.reserve(e.inputs.size());
    std::vector<Expr*> passthroughTerms;
    passthroughTerms.reserve(e.inputs.size());

    for (Expr* input : e.inputs) {
        LinearTerm linear{};
        if (!DecomposeLinearTerm(input, linear)) {
            passthroughTerms.push_back(input);
            continue;
        }

        const uint32_t baseId = linear.base->id.value();
        if (coeffByBaseId.emplace(baseId, 0u).second) {
            baseOrder.push_back(baseId);
            baseExprById[baseId] = linear.base;
        }
        coeffByBaseId[baseId] += linear.coeff;
    }

    std::vector<Expr*> normalizedAddTerms;
    normalizedAddTerms.reserve(baseOrder.size() + passthroughTerms.size());
    for (uint32_t baseId : baseOrder) {
        Expr* base = baseExprById[baseId];
        const uint32_t coeff = coeffByBaseId[baseId];
        if (coeff == 0u)
            continue;
        if (coeff == 1u) {
            normalizedAddTerms.push_back(base);
            continue;
        }
        normalizedAddTerms.push_back(MakeOpInterned(OpType::Mul, {ConstPool::Get(coeff), base}));
    }
    for (Expr* term : passthroughTerms)
        normalizedAddTerms.push_back(term);

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
