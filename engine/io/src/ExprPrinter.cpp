#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/io/ExprPrinter.h>
#include <sstream>

namespace BitFlow::IO {

using Expr = Core::AST::Expr;
using OpType = Core::AST::OpType;

struct InfixInfo {
    int precedence = 0;
    const char* symbol = nullptr;
};

static bool TryGetInfixInfo(OpType op, InfixInfo& info) {
    switch (op) {
    case OpType::Mul:
        info = InfixInfo{60, "*"};
        return true;
    case OpType::Div:
        info = InfixInfo{60, "/"};
        return true;
    case OpType::Mod:
        info = InfixInfo{60, "%"};
        return true;
    case OpType::Add:
        info = InfixInfo{50, "+"};
        return true;
    case OpType::Sub:
        info = InfixInfo{50, "-"};
        return true;
    case OpType::Shl:
        info = InfixInfo{40, "<<"};
        return true;
    case OpType::Shr:
        info = InfixInfo{40, ">>"};
        return true;
    case OpType::UShr:
        info = InfixInfo{40, ">>>"};
        return true;
    case OpType::And:
        info = InfixInfo{30, "&"};
        return true;
    case OpType::Xor:
        info = InfixInfo{20, "^"};
        return true;
    case OpType::Or:
        info = InfixInfo{10, "|"};
        return true;
    default:
        return false;
    }
}

static int PrecedenceOf(const Expr* e) {
    if (e->isConst() || e->op == OpType::Var || e->op == OpType::RotL || e->op == OpType::RotR || e->op == OpType::Ch ||
        e->op == OpType::Maj)
        return 80;

    if (e->op == OpType::Not || e->op == OpType::Neg)
        return 70;

    InfixInfo info{};
    if (TryGetInfixInfo(e->op, info))
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

static void Print(const Expr* e, std::ostringstream& out, const std::unordered_map<uint32_t, std::string>& names,
                  int parentPrecedence, bool isRightChild, OpType parentOp) {
    if (e->isConst()) {
        out << e->constValue;
        return;
    }

    if (e->op == OpType::Var) {
        auto it = names.find(e->id.value());
        if (it != names.end())
            out << it->second;
        else
            out << "v" << e->id.value();
        return;
    }

    if (e->op == OpType::Not || e->op == OpType::Neg) {
        out << (e->op == OpType::Not ? "~" : "-");
        const Expr* inner = e->inputs[0];
        const bool needsParens = PrecedenceOf(inner) < PrecedenceOf(e);
        if (needsParens)
            out << "(";
        Print(inner, out, names, PrecedenceOf(e), true, e->op);
        if (needsParens)
            out << ")";
        return;
    }

    if (e->op == OpType::RotL || e->op == OpType::RotR) {
        out << (e->op == OpType::RotL ? "rotl(" : "rotr(");
        Print(e->inputs[0], out, names, 0, false, OpType::Var);
        out << ", ";
        Print(e->inputs[1], out, names, 0, true, OpType::Var);
        out << ")";
        return;
    }

    if (e->op == OpType::Ch || e->op == OpType::Maj) {
        out << (e->op == OpType::Ch ? "ch(" : "maj(");
        Print(e->inputs[0], out, names, 0, false, OpType::Var);
        out << ", ";
        Print(e->inputs[1], out, names, 0, false, OpType::Var);
        out << ", ";
        Print(e->inputs[2], out, names, 0, true, OpType::Var);
        out << ")";
        return;
    }

    InfixInfo info{};
    const bool isInfix = TryGetInfixInfo(e->op, info);
    const int currentPrecedence = isInfix ? info.precedence : 0;
    bool wrapSelf = false;
    if (currentPrecedence < parentPrecedence)
        wrapSelf = true;
    else if (isRightChild && currentPrecedence == parentPrecedence)
        wrapSelf = NeedsParensForRightChild(parentOp, e->op);

    if (wrapSelf)
        out << "(";
    for (size_t i = 0; i < e->inputs.size(); ++i) {
        if (i > 0)
            out << " " << info.symbol << " ";

        Print(e->inputs[i], out, names, currentPrecedence, i > 0, e->op);
    }
    if (wrapSelf)
        out << ")";
}

std::string ToString(const Expr* e) {
    std::ostringstream out;
    Print(e, out, {}, 0, false, OpType::Var);
    return out.str();
}

std::string ToString(const Expr* e, const std::unordered_map<uint32_t, std::string>& names) {
    std::ostringstream out;
    Print(e, out, names, 0, false, OpType::Var);
    return out.str();
}

} // namespace BitFlow::IO
