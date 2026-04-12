// #include "rules/RuleStage.h"
//
// #include <BitFlow/core/ast/Expression.h>
// #include <BitFlow/core/rules/Rule.h>
// #include <unordered_set>
//
// namespace BitFlow::Core::Rules::Simplify {
//
// using Expr = AST::Expr;
//
// static bool Match(const Expr& e) {
//     if (e.op != AST::OpType::And || e.inputs.size() < 2)
//         return false;
//
//     std::unordered_set<const Expr*> seen;
//
//     for (auto* in : e.inputs) {
//         if (!seen.insert(in).second)
//             return true;
//     }
//     return false;
// }
//
// static Expr* Rewrite(Expr& e) {
//     std::unordered_set<const Expr*> seen;
//     std::vector<Expr*> unique;
//
//     unique.reserve(e.inputs.size());
//
//     for (auto* in : e.inputs) {
//         if (seen.insert(in).second)
//             unique.push_back(in);
//     }
//
//     if (unique.size() == 1)
//         return unique[0];
//
//     e.inputs = std::move(unique);
//     return &e;
// }
//
// Rule Get_And_Idempotent_Rule() {
//     return Rule{RuleId::Simplify_And_Idempotent, &Match, &Rewrite, Stage_Simplify, {RuleId::Normalize_Flatten}};
// }
//
// } // namespace BitFlow::Core::Rules::Simplify