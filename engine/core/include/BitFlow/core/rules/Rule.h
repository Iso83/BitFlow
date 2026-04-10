#pragma once

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {

struct Rule {
    bool (*match)(const AST::Expr&);
    AST::Expr* (*rewrite)(AST::Expr&);
    int stage;
};

namespace Normalize {
Rule Get_Order_Rule();
Rule Get_Flatten_Rule();
} // namespace Normalize

namespace Simplify {
Rule Get_Add_Zero_Rule();
Rule Get_Xor_Zero_Rule();
Rule Get_And_Fold_Rule();
Rule Get_Or_Fold_Rule();
Rule Get_Xor_Fold_Rule();
Rule Get_Xor_Cancel_Rule();
} // namespace Simplify

namespace Factorize {
Rule Get_Xor_And_Rule();
Rule Get_Xor_Pair_Cancel_Rule();
} // namespace Factorize

} // namespace BitFlow::Core::Rules