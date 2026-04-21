#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/RewriteCost.h>
#include <tuple>

namespace BitFlow::Core::Rules {
namespace {

using Expr = AST::Expr;
using OpType = AST::OpType;

struct CostAccumulator {
    RewriteCost cost{};

    void Visit(const Expr* expr, uint32_t depth) {
        if (expr == nullptr)
            return;

        cost.totalNodes += 1U;
        if (!AST::IsLeaf(expr->op))
            cost.operatorNodes += 1U;
        if (depth > cost.maxDepth)
            cost.maxDepth = depth;

        if ((expr->op == OpType::And || expr->op == OpType::Mul) && expr->inputs.size() >= 2) {
            const OpType distributiveInner = (expr->op == OpType::And) ? OpType::Xor : OpType::Add;
            for (const Expr* in : expr->inputs) {
                if (in->op == distributiveInner && in->inputs.size() >= 2) {
                    cost.distributivePatterns += 1U;
                    break;
                }
            }
        }

        if (AST::IsAssociative(expr->op)) {
            for (const Expr* in : expr->inputs) {
                if (in->op == expr->op)
                    cost.nestedAssociativeNodes += 1U;
            }
        }

        for (const Expr* in : expr->inputs)
            Visit(in, depth + 1U);
    }
};

} // namespace

RewriteCost ComputeRewriteCost(const AST::Expr* expr) {
    CostAccumulator acc{};
    acc.Visit(expr, 1U);
    return acc.cost;
}

bool IsRewritePreferred(const AST::Expr* candidate, const AST::Expr* current, RewriteCostPolicy policy) {
    if (candidate == nullptr || current == nullptr)
        return false;

    const RewriteCost next = ComputeRewriteCost(candidate);
    const RewriteCost prev = ComputeRewriteCost(current);

    switch (policy) {
    case RewriteCostPolicy::FactorizeSafe:
        return std::tie(next.totalNodes, next.operatorNodes, next.maxDepth, next.distributivePatterns,
                        next.nestedAssociativeNodes) <= std::tie(prev.totalNodes, prev.operatorNodes, prev.maxDepth,
                                                                 prev.distributivePatterns,
                                                                 prev.nestedAssociativeNodes);
    case RewriteCostPolicy::ExpandDistribute:
        return std::tie(next.distributivePatterns, next.totalNodes, next.operatorNodes, next.maxDepth,
                        next.nestedAssociativeNodes) <= std::tie(prev.distributivePatterns, prev.totalNodes,
                                                                 prev.operatorNodes, prev.maxDepth,
                                                                 prev.nestedAssociativeNodes);
    default:
        return false;
    }
}

} // namespace BitFlow::Core::Rules
