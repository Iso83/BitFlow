#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static ExprId FindBestCommonFactor(const ExprStore* store, ExprId id, OpType termOp = OpType::And) {
    const Expr& e = (*store)[id];

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != termOp || term.inputs.size() < 2)
            continue;

        ExprInputs seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);
            counts[inId]++;
        }
    }

    ExprId best;
    size_t bestCount = 0;
    bool hasBest = false;

    for (const auto& [factorId, count] : counts) {
        if (count < 2)
            continue;

        if (!hasBest || count > bestCount) {
            best = factorId;
            bestCount = count;
            hasBest = true;
        }
    }

    if (!hasBest) {
        BF_CORE_ASSERT(false);
        return id;
    }

    return best;
}

#pragma region Match
static bool Match_XorAnd(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        ExprInputs seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);

            auto& cnt = counts[inId];
            if (++cnt >= 2)
                return true;
        }
    }

    return false;
}

static bool Match_DistributeAndOverOr(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Or || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        ExprInputs seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);

            auto& cnt = counts[inId];
            if (++cnt >= 2)
                return true;
        }
    }

    return false;
}

static bool Match_DistributeOrOverAnd(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And || e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, size_t> counts;

    for (auto termId : e.inputs) {
        const Expr& term = (*store)[termId];

        if (term.op != OpType::Or || term.inputs.size() < 2)
            continue;

        ExprInputs seen;
        seen.reserve(term.inputs.size());

        for (auto inId : term.inputs) {
            if (std::find(seen.begin(), seen.end(), inId) != seen.end())
                continue;

            seen.push_back(inId);

            auto& cnt = counts[inId];
            if (++cnt >= 2)
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_XorAnd(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr e = (*store)[id];

    const ExprId bestFactor = FindBestCommonFactor(store, id);

    ExprInputs termsToFactor;
    termsToFactor.reserve(e.inputs.size());

    for (auto termId : e.inputs) {
        const Expr term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        for (auto inId : term.inputs) {
            if (inId == bestFactor) {
                termsToFactor.push_back(termId);
                break;
            }
        }
    }

    if (termsToFactor.size() < 2) {
        BF_CORE_ASSERT(false);
        return id;
    }

    ExprInputs xorInputs;
    xorInputs.reserve(termsToFactor.size());

    for (auto termId : termsToFactor) {
        const Expr term = (*store)[termId];

        ExprInputs rest;
        rest.reserve(term.inputs.size());

        bool removed = false;

        for (auto inId : term.inputs) {
            if (!removed && inId == bestFactor) {
                removed = true;
                continue;
            }

            rest.push_back(inId);
        }

        if (rest.empty()) {
            xorInputs.push_back(store->makeTrue(term.bitWidth).id);
        } else if (rest.size() == 1) {
            xorInputs.push_back(rest[0]);
        } else {
            xorInputs.push_back(store->create(OpType::And, std::move(rest), term.bitWidth).id);
        }
    }

    ExprId xorExpr{};

    const Types::BitWidth bitWidth = e.bitWidth;
    const ExprInputs inputs = e.inputs;

    if (xorInputs.size() == 1) {
        xorExpr = xorInputs[0];
    } else {
        xorExpr = store->create(OpType::Xor, std::move(xorInputs), bitWidth).id;
    }

    ExprId factored = store->create(OpType::And, {bestFactor, xorExpr}, bitWidth).id;

    ExprInputs finalInputs;
    finalInputs.reserve(inputs.size() - termsToFactor.size() + 1);

    finalInputs.push_back(factored);

    for (auto termId : inputs) {
        if (std::find(termsToFactor.begin(), termsToFactor.end(), termId) == termsToFactor.end()) {
            finalInputs.push_back(termId);
        }
    }

    if (finalInputs.size() == 1)
        return ctx.replace(id, finalInputs[0]);

    return ctx.replace(id, store->create(OpType::Xor, std::move(finalInputs), bitWidth).id);
}

static ExprId Rewrite_DistributeAndOverOr(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr e = (*store)[id];

    const ExprId bestFactor = FindBestCommonFactor(store, id);

    ExprInputs termsToFactor;
    termsToFactor.reserve(e.inputs.size());

    for (auto termId : e.inputs) {
        const Expr term = (*store)[termId];

        if (term.op != OpType::And || term.inputs.size() < 2)
            continue;

        for (auto inId : term.inputs) {
            if (inId == bestFactor) {
                termsToFactor.push_back(termId);
                break;
            }
        }
    }

    if (termsToFactor.size() < 2) {
        BF_CORE_ASSERT(false);
        return id;
    }

    ExprInputs orInputs;
    orInputs.reserve(termsToFactor.size());

    for (auto termId : termsToFactor) {
        const Expr term = (*store)[termId];

        ExprInputs rest;
        rest.reserve(term.inputs.size());

        bool removed = false;

        for (auto inId : term.inputs) {
            if (!removed && inId == bestFactor) {
                removed = true;
                continue;
            }

            rest.push_back(inId);
        }

        if (rest.empty())
            orInputs.push_back(store->makeTrue(term.bitWidth).id);
        else if (rest.size() == 1)
            orInputs.push_back(rest[0]);
        else
            orInputs.push_back(store->create(OpType::And, std::move(rest), term.bitWidth).id);
    }

    ExprId orExpr{};

    const Types::BitWidth bitWidth = e.bitWidth;
    const ExprInputs inputs = e.inputs;

    if (orInputs.size() == 1)
        orExpr = orInputs[0];
    else
        orExpr = store->create(OpType::Or, std::move(orInputs), bitWidth).id;

    ExprId factored = store->create(OpType::And, {bestFactor, orExpr}, bitWidth).id;

    ExprInputs finalInputs;
    finalInputs.reserve(inputs.size() - termsToFactor.size() + 1);

    finalInputs.push_back(factored);

    for (auto termId : inputs) {
        if (std::find(termsToFactor.begin(), termsToFactor.end(), termId) == termsToFactor.end())
            finalInputs.push_back(termId);
    }

    if (finalInputs.size() == 1)
        return ctx.replace(id, finalInputs[0]);

    return ctx.replace(id, store->create(OpType::Or, std::move(finalInputs), bitWidth).id);
}

static ExprId Rewrite_DistributeOrOverAnd(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr e = (*store)[id];

    const ExprId bestFactor = FindBestCommonFactor(store, id, OpType::Or);

    ExprInputs termsToFactor;
    termsToFactor.reserve(e.inputs.size());

    for (auto termId : e.inputs) {
        const Expr term = (*store)[termId];

        if (term.op != OpType::Or || term.inputs.size() < 2)
            continue;

        for (auto inId : term.inputs) {
            if (inId == bestFactor) {
                termsToFactor.push_back(termId);
                break;
            }
        }
    }

    if (termsToFactor.size() < 2) {
        BF_CORE_ASSERT(false);
        return id;
    }

    ExprInputs andInputs;
    andInputs.reserve(termsToFactor.size());

    for (auto termId : termsToFactor) {
        const Expr term = (*store)[termId];

        ExprInputs rest;
        rest.reserve(term.inputs.size());

        bool removed = false;

        for (auto inId : term.inputs) {
            if (!removed && inId == bestFactor) {
                removed = true;
                continue;
            }

            rest.push_back(inId);
        }

        if (rest.empty())
            andInputs.push_back(store->makeFalse(term.bitWidth).id);
        else if (rest.size() == 1)
            andInputs.push_back(rest[0]);
        else
            andInputs.push_back(store->create(OpType::Or, std::move(rest), term.bitWidth).id);
    }

    ExprId andExpr{};

    const Types::BitWidth bitWidth = e.bitWidth;
    const ExprInputs inputs = e.inputs;

    if (andInputs.size() == 1)
        andExpr = andInputs[0];
    else
        andExpr = store->create(OpType::And, std::move(andInputs), bitWidth).id;

    ExprId factored = store->create(OpType::Or, {bestFactor, andExpr}, bitWidth).id;

    ExprInputs finalInputs;
    finalInputs.reserve(inputs.size() - termsToFactor.size() + 1);

    finalInputs.push_back(factored);

    for (auto termId : inputs) {
        if (std::find(termsToFactor.begin(), termsToFactor.end(), termId) == termsToFactor.end())
            finalInputs.push_back(termId);
    }

    if (finalInputs.size() == 1)
        return ctx.replace(id, finalInputs[0]);

    return ctx.replace(id, store->create(OpType::And, std::move(finalInputs), bitWidth).id);
}
#pragma endregion

Rule Get_XorAnd_Rule() {
    return Rule{XorAnd, &Match_XorAnd, &Rewrite_XorAnd, {Simplify::Bitwise::XorAndReduction}};
}

Rule Get_DistributeAndOverOr_Rule() {
    return Rule{DistributeAndOverOr, &Match_DistributeAndOverOr, &Rewrite_DistributeAndOverOr, {Normalize::Flatten}};
}

Rule Get_DistributeOrOverAnd_Rule() {
    return Rule{DistributeOrOverAnd, &Match_DistributeOrOverAnd, &Rewrite_DistributeOrOverAnd, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise
