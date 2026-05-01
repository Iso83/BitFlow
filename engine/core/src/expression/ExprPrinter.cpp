#include "expression/ExprPrinter.h"

#include <BitFlow/core/expression/Expression.h>
#include <sstream>

namespace BitFlow::Core::Expression {

struct InfixInfo {
    int precedence = 0;
    const char* symbol = nullptr;
};

static bool TryGetInfixInfo(OpType op, InfixInfo& info) {
    switch (op) {
    case OpType::Mul:
        info = {60, "*"};
        return true;
    case OpType::Div:
        info = {60, "/"};
        return true;
    case OpType::Mod:
        info = {60, "%"};
        return true;
    case OpType::Add:
        info = {50, "+"};
        return true;
    case OpType::Sub:
        info = {50, "-"};
        return true;
    case OpType::Shl:
        info = {40, "<<"};
        return true;
    case OpType::Shr:
        info = {40, ">>"};
        return true;
    case OpType::And:
        info = {30, "&"};
        return true;
    case OpType::Xor:
        info = {20, "^"};
        return true;
    case OpType::Or:
        info = {10, "|"};
        return true;
    default:
        return false;
    }
}

static int PrecedenceOf(const Expr& e) {
    if (e.op == OpType::Const || e.op == OpType::Var || e.op == OpType::RotL || e.op == OpType::RotR ||
        e.op == OpType::Ch || e.op == OpType::Maj)
        return 80;

    if (e.op == OpType::Not || e.op == OpType::Neg)
        return 70;

    InfixInfo info{};
    if (TryGetInfixInfo(e.op, info))
        return info.precedence;

    return 0;
}

static bool NeedsParensForRightChild(OpType parentOp, OpType childOp) {
    if (parentOp == OpType::Add)
        return childOp != OpType::Add;

    if (parentOp == OpType::Mul)
        return childOp != OpType::Mul;

    if (parentOp == OpType::And || parentOp == OpType::Or || parentOp == OpType::Xor)
        return false;

    return true;
}

static void Print(const ExprStore* store, const Expr& e, std::ostringstream& out,
                  const std::unordered_map<Ids::ExprId, std::string>& names, const PrintOptions& options,
                  int parentPrecedence, bool isRightChild, OpType parentOp) {
    if (e.op == OpType::Const) {
        out << e.knownValue;
        return;
    }

    if (e.op == OpType::Var) {
        auto it = names.find(e.id);
        if (it != names.end())
            out << it->second;
        else
            out << "v" << e.id.value();
        return;
    }

    if (e.op == OpType::Not || e.op == OpType::Neg) {
        out << (e.op == OpType::Not ? "~" : "-");

        const Expr& inner = store->get(e.inputs[0]);
        const bool needsParens = PrecedenceOf(inner) < PrecedenceOf(e);

        if (needsParens) {
            out << "(";
            Print(store, inner, out, names, options, 0, false, OpType::Var);
            out << ")";
        } else {
            Print(store, inner, out, names, options, PrecedenceOf(e), true, e.op);
        }

        return;
    }

    if (e.op == OpType::RotL || e.op == OpType::RotR) {
        if (options.rotAsFunction) {
            out << (e.op == OpType::RotL ? "rotl(" : "rotr(");
            Print(store, store->get(e.inputs[0]), out, names, options, 0, false, OpType::Var);
            out << ", ";
            Print(store, store->get(e.inputs[1]), out, names, options, 0, true, OpType::Var);
            out << ")";
            return;
        }

        const int currentPrecedence = 40;
        bool wrapSelf = false;

        if (currentPrecedence < parentPrecedence)
            wrapSelf = true;
        else if (isRightChild && currentPrecedence == parentPrecedence)
            wrapSelf = NeedsParensForRightChild(parentOp, e.op);

        if (wrapSelf)
            out << "(";

        Print(store, store->get(e.inputs[0]), out, names, options, currentPrecedence, false, e.op);
        out << (e.op == OpType::RotL ? " <<< " : " >>> ");
        Print(store, store->get(e.inputs[1]), out, names, options, currentPrecedence, true, e.op);

        if (wrapSelf)
            out << ")";

        return;
    }

    if (e.op == OpType::Ch || e.op == OpType::Maj) {
        out << (e.op == OpType::Ch ? "ch(" : "maj(");
        Print(store, store->get(e.inputs[0]), out, names, options, 0, false, OpType::Var);
        out << ", ";
        Print(store, store->get(e.inputs[1]), out, names, options, 0, false, OpType::Var);
        out << ", ";
        Print(store, store->get(e.inputs[2]), out, names, options, 0, true, OpType::Var);
        out << ")";
        return;
    }

    InfixInfo info{};
    const bool isInfix = TryGetInfixInfo(e.op, info);
    const int currentPrecedence = isInfix ? info.precedence : 0;

    bool wrapSelf = false;

    if (currentPrecedence < parentPrecedence)
        wrapSelf = true;
    else if (isRightChild && currentPrecedence == parentPrecedence)
        wrapSelf = NeedsParensForRightChild(parentOp, e.op);

    if (wrapSelf)
        out << "(";

    for (std::size_t i = 0; i < e.inputs.size(); ++i) {
        if (i > 0)
            out << " " << info.symbol << " ";

        Print(store, store->get(e.inputs[i]), out, names, options, currentPrecedence, i > 0, e.op);
    }

    if (wrapSelf)
        out << ")";
}

std::string ToString(const ExprStore* store, const Ids::ExprId e) {
    std::ostringstream out;
    Print(store, store->get(e), out, {}, PrintOptions{}, 0, false, OpType::Var);
    return out.str();
}

std::string ToString(const ExprStore* store, const Ids::ExprId e,
                     const std::unordered_map<Ids::ExprId, std::string>& names) {
    std::ostringstream out;
    Print(store, store->get(e), out, names, PrintOptions{}, 0, false, OpType::Var);
    return out.str();
}

std::string ToString(const ExprStore* store, const Ids::ExprId e,
                     const std::unordered_map<Ids::ExprId, std::string>& names, const PrintOptions& options) {
    std::ostringstream out;
    Print(store, store->get(e), out, names, options, 0, false, OpType::Var);
    return out.str();
}

} // namespace BitFlow::Core::Expression