#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>

namespace BitFlow::Core {

#pragma region Match
static bool Match_Xor_And_CommonFactor(const Expr& e) {
    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* left = e.inputs[0];
    Expr* right = e.inputs[1];

    if (left->op != OpType::And || right->op != OpType::And)
        return false;

    if (left->inputs.size() != 2 || right->inputs.size() != 2)
        return false;

    Expr* a0 = left->inputs[0];
    Expr* a1 = left->inputs[1];
    Expr* b0 = right->inputs[0];
    Expr* b1 = right->inputs[1];

    return (a0->id == b0->id) || (a0->id == b1->id) || (a1->id == b0->id) || (a1->id == b1->id);
}

static bool Match_Xor_Xor_CancelPair(const Expr& e) {
    if (e.op != OpType::Xor)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* l = e.inputs[0];
    Expr* r = e.inputs[1];

    if (l->op != OpType::Xor || r->op != OpType::Xor)
        return false;

    if (l->inputs.size() != 2 || r->inputs.size() != 2)
        return false;

    Expr* a0 = l->inputs[0];
    Expr* a1 = l->inputs[1];
    Expr* b0 = r->inputs[0];
    Expr* b1 = r->inputs[1];

    return (a0->id == b0->id) || (a0->id == b1->id) || (a1->id == b0->id) || (a1->id == b1->id);
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Xor_And_CommonFactor(Expr& e) {
    Expr* left = e.inputs[0];
    Expr* right = e.inputs[1];

    Expr* a0 = left->inputs[0];
    Expr* a1 = left->inputs[1];
    Expr* b0 = right->inputs[0];
    Expr* b1 = right->inputs[1];

    Expr* common = nullptr;
    Expr* otherLeft = nullptr;
    Expr* otherRight = nullptr;

    if (a0->id == b0->id) {
        common = a0;
        otherLeft = a1;
        otherRight = b1;
    } else if (a0->id == b1->id) {
        common = a0;
        otherLeft = a1;
        otherRight = b0;
    } else if (a1->id == b0->id) {
        common = a1;
        otherLeft = a0;
        otherRight = b1;
    } else {
        common = a1;
        otherLeft = a0;
        otherRight = b0;
    }

    Expr* innerXor = new Expr{};
    innerXor->op = OpType::Xor;
    innerXor->inputs = {otherLeft, otherRight};

    Expr* result = new Expr{};
    result->op = OpType::And;
    result->inputs = {common, innerXor};

    return result;
}

static Expr* Rewrite_Xor_Xor_CancelPair(Expr& e) {
    Expr* l = e.inputs[0];
    Expr* r = e.inputs[1];

    Expr* a0 = l->inputs[0];
    Expr* a1 = l->inputs[1];
    Expr* b0 = r->inputs[0];
    Expr* b1 = r->inputs[1];

    Expr* otherL = nullptr;
    Expr* otherR = nullptr;

    if (a0->id == b0->id) {
        otherL = a1;
        otherR = b1;
    } else if (a0->id == b1->id) {
        otherL = a1;
        otherR = b0;
    } else if (a1->id == b0->id) {
        otherL = a0;
        otherR = b1;
    } else {
        otherL = a0;
        otherR = b0;
    }

    Expr* n = new Expr{};
    n->op = OpType::Xor;
    n->inputs = {otherL, otherR};
    return n;
}
#pragma endregion

Rule Get_Xor_And_CommonFactor_Rule() {
    return Rule{&Match_Xor_And_CommonFactor, &Rewrite_Xor_And_CommonFactor};
}

Rule Get_Xor_Xor_CancelPair_Rule() {
    return Rule{&Match_Xor_Xor_CancelPair, &Rewrite_Xor_Xor_CancelPair};
}

} // namespace BitFlow::Core