#pragma once

namespace BitFlow::Core {
struct Expr;

struct Rule {
    bool (*match)(const Expr&);
    Expr* (*rewrite)(Expr&);
};

Rule Get_Add_Zero_Rule();
Rule Get_Xor_Zero_Rule();

} // namespace BitFlow::Core
