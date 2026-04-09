#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>

namespace BitFlow::Core {

// ---------- GENERIC ----------

template <OpType Op> static bool Match_Zero(const Expr& e) {
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (in->isConst && in->constValue == 0)
            return true;
    }

    return false;
}

static Expr* Rewrite_Remove_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!(in->isConst && in->constValue == 0)) 
            newInputs.push_back(in);
    }

    // alles was 0 → resultaat = 0
    if (newInputs.empty()) 
        return e.inputs[0]; // e.g. 0 + 0 → 0 (mag eerste nemen)

    // 1 element over → collapsen
    if (newInputs.size() == 1)
        return newInputs[0];

    // anders inputs vervangen
    e.inputs = std::move(newInputs);
    return &e;
}

// ---------- ADD ----------

Rule Get_Add_Zero_Rule() {
    return Rule{&Match_Zero<OpType::Add>, &Rewrite_Remove_Zero};
}

// ---------- XOR ----------

Rule Get_Xor_Zero_Rule() {
    return Rule{&Match_Zero<OpType::Xor>, &Rewrite_Remove_Zero};
}

} // namespace BitFlow::Core