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
// --- ORDER ---
Rule Get_Normalize_Order_Rule(); // was: Get_Order_Rule

// --- FLATTEN ---
Rule Get_Normalize_Flatten_Rule(); // was: Get_Flatten_Rule
} // namespace Normalize

namespace Simplify {
// --- ZERO ---
Rule Get_Simplify_Add_Zero_Rule(); // was: Get_Add_Zero_Rule
Rule Get_Simplify_Xor_Zero_Rule(); // was: Get_Xor_Zero_Rule

// --- FOLD ---
Rule Get_Simplify_And_Fold_Rule(); // was: Get_And_Fold_Rule
Rule Get_Simplify_Or_Fold_Rule();  // was: Get_Or_Fold_Rule
Rule Get_Simplify_Xor_Fold_Rule(); // was: Get_Xor_Fold_Rule

// --- CANCEL ---
Rule Get_Simplify_Xor_Cancel_Rule(); // was: Get_Xor_Cancel_Rule
} // namespace Simplify

namespace Factorize {
// --- FACTORIZE ---
Rule Get_Factorize_Xor_And_Rule(); // was: Get_Xor_And_CommonFactor_Rule

// --- CANCEL (PAIR / CROSS) ---
Rule Get_Factorize_Xor_Pair_Cancel_Rule(); // was: Get_Xor_Xor_CancelPair_Rule
} // namespace Factorize

} // namespace BitFlow::Core::Rules