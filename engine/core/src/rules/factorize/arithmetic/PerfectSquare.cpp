#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <cmath>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

namespace {

struct SignedTerm {
    ExprId id{};
    bool negative{false};
};

bool CollectSignedTerms(const ExprStore* store, ExprId id, bool negated, std::vector<SignedTerm>& out) {
    const Expr& e = (*store)[id];

    if (e.op == OpType::Add) {
        for (ExprId in : e.inputs)
            if (!CollectSignedTerms(store, in, negated, out))
                return false;
        return true;
    }

    if (e.op == OpType::Sub && e.inputs.size() == 2) {
        if (!CollectSignedTerms(store, e.inputs[0], negated, out))
            return false;
        if (!CollectSignedTerms(store, e.inputs[1], !negated, out))
            return false;
        return true;
    }

    out.push_back({id, negated});
    return true;
}

bool TryGetSquareRoot(const ExprStore* store, ExprId termId, ExprId& rootOut) {
    const Expr& term = (*store)[termId];
    if (term.op == OpType::Pow && term.inputs.size() == 2) {
        const Expr& exp = (*store)[term.inputs[1]];
        if (exp.op == OpType::Const && exp.knownValue == 2) {
            rootOut = term.inputs[0];
            return true;
        }
    }

    if (term.op == OpType::Const) {
        const auto v = term.knownValue;
        const auto r = static_cast<Types::ExprChunk>(std::llround(std::sqrt(static_cast<long double>(v))));
        if (r * r == v) {
            rootOut = const_cast<ExprStore*>(store)->createConstant(r, term.bitWidth).id;
            return true;
        }
    }

    return false;
}

void ParseCrossTerm(const ExprStore* store, ExprId termId, Types::ExprChunk& coeffOut, ExprInputs& factorsOut,
                    Types::BitWidth bitWidth) {
    const Expr& term = (*store)[termId];
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    coeffOut = 1;
    factorsOut.clear();

    if (term.op == OpType::Mul) {
        for (ExprId in : term.inputs) {
            const Expr& factor = (*store)[in];
            if (factor.op == OpType::Const)
                coeffOut = (coeffOut * factor.knownValue) & mask;
            else
                factorsOut.push_back(in);
        }
        return;
    }

    if (term.op == OpType::Const) {
        coeffOut = term.knownValue;
        return;
    }

    factorsOut.push_back(termId);
}

bool MatchCrossTermForRoots(const ExprStore* store, ExprId r1, ExprId r2, Types::ExprChunk observedCoeff,
                            const ExprInputs& observedFactors, Types::BitWidth bitWidth, bool& negativeOut) {
    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    Types::ExprChunk expectedCoeff = 2 & mask;
    ExprInputs expectedFactors;

    for (ExprId rootId : {r1, r2}) {
        const Expr& root = (*store)[rootId];
        if (root.op == OpType::Const)
            expectedCoeff = (expectedCoeff * root.knownValue) & mask;
        else
            expectedFactors.push_back(rootId);
    }

    if (expectedFactors.size() != observedFactors.size())
        return false;

    for (ExprId ex : expectedFactors) {
        bool found = false;
        for (ExprId f : observedFactors) {
            if (store->structuralEquivalent(ex, f)) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }

    if (observedCoeff == expectedCoeff) {
        negativeOut = false;
        return true;
    }

    if (observedCoeff == ((static_cast<Types::ExprChunk>(0) - expectedCoeff) & mask)) {
        negativeOut = true;
        return true;
    }

    return false;
}

bool TryMatch(const ExprStore* store, ExprId id, ExprId& root1Out, ExprId& root2Out, bool& negativeCrossOut) {
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    if (e.op != OpType::Add && e.op != OpType::Sub)
        return false;

    std::vector<SignedTerm> terms;
    if (!CollectSignedTerms(store, id, false, terms) || terms.size() != 3)
        return false;

    const Types::ExprChunk mask = Expr::fullMask(bitWidth);

    for (size_t i = 0; i < terms.size(); ++i) {
        for (size_t j = i + 1; j < terms.size(); ++j) {
            if (terms[i].negative || terms[j].negative)
                continue;

            ExprId r1{}, r2{};
            if (!TryGetSquareRoot(store, terms[i].id, r1) || !TryGetSquareRoot(store, terms[j].id, r2))
                continue;

            const size_t k = 3 - i - j;
            Types::ExprChunk coeff{};
            ExprInputs factors;
            ParseCrossTerm(store, terms[k].id, coeff, factors, bitWidth);

            if (terms[k].negative)
                coeff = (static_cast<Types::ExprChunk>(0) - coeff) & mask;

            bool negativeCross = false;
            if (!MatchCrossTermForRoots(store, r1, r2, coeff, factors, bitWidth, negativeCross))
                continue;

            root1Out = r1;
            root2Out = r2;
            negativeCrossOut = negativeCross;
            return true;
        }
    }

    return false;
}

} // namespace

static bool Match_PerfectSquare(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    ExprId r1{}, r2{};
    bool neg = false;
    return TryMatch(store, id, r1, r2, neg);
}

static ExprId Rewrite_PerfectSquare(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    ExprId r1{}, r2{};
    bool neg = false;
    if (!TryMatch(store, id, r1, r2, neg))
        return id;

    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    ExprId inner =
        neg ? store->create(OpType::Sub, {r1, r2}, bitWidth).id : store->create(OpType::Add, {r1, r2}, bitWidth).id;
    ExprId two = store->createConstant(2, bitWidth).id;
    return ctx.replace(id, store->create(OpType::Pow, {inner, two}, bitWidth).id);
}

Rule Get_PerfectSquare_Rule() {
    return Rule{PerfectSquare, &Match_PerfectSquare, &Rewrite_PerfectSquare, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
