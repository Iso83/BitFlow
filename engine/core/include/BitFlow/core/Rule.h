#pragma once

namespace BitFlow::Core {
struct Expr;

struct Rule {
    bool (*match)(const Expr&);
    Expr* (*rewrite)(Expr&);
};

// --- ORDER ---
Rule Get_Order_Rule();

// --- ZERO ---
Rule Get_Add_Zero_Rule();
Rule Get_Xor_Zero_Rule();

// --- FLATTEN ---
Rule Get_Flatten_Rule();

// --- FOLD ---
Rule Get_And_Fold_Rule();
Rule Get_Or_Fold_Rule();
Rule Get_Xor_Fold_Rule();

// --- XOR ---
Rule Get_Xor_Cancel_Rule();

} // namespace BitFlow::Core
