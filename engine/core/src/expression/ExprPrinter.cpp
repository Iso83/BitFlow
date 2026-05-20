#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/expression/OpInfo.h>
#include <sstream>

namespace BitFlow::Core::Expression {

static const char* OpTypeName(OpType op) {
    switch (op) {
    case OpType::Var:
        return "Var";
    case OpType::Const:
        return "Const";
    case OpType::Not:
        return "Not";
    case OpType::Neg:
        return "Neg";

    case OpType::And:
        return "And";
    case OpType::Or:
        return "Or";
    case OpType::Xor:
        return "Xor";

    case OpType::Add:
        return "Add";
    case OpType::Sub:
        return "Sub";
    case OpType::Mul:
        return "Mul";
    case OpType::Div:
        return "Div";
    case OpType::Mod:
        return "Mod";

    case OpType::Pow:
        return "Pow";

    case OpType::Shl:
        return "Shl";
    case OpType::Shr:
        return "Shr";

    case OpType::RotL:
        return "RotL";
    case OpType::RotR:
        return "RotR";

    default:
        return "Unknown";
    }
}

static int PrecedenceOf(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op == OpType::Const || e.op == OpType::Var || e.op == OpType::RotL || e.op == OpType::RotR)
        return 80;

    if (e.op == OpType::Not || e.op == OpType::Neg)
        return 70;

    const OpInfo* info = GetOpInfo(e.op);
    if (info)
        return info->precedence;

    return 0;
}

static bool NeedsParensForRightChild(OpType parentOp, OpType childOp) {
    if (parentOp == OpType::Add)
        return childOp != OpType::Add;

    if (parentOp == OpType::Mul)
        return childOp != OpType::Mul;

    if (parentOp == OpType::And || parentOp == OpType::Or || parentOp == OpType::Xor)
        return false;

    if (parentOp == OpType::Shl || parentOp == OpType::Shr)
        return childOp == OpType::Add || childOp == OpType::Sub;

    return true;
}

static void PrintDebugStructure(const ExprStore* store, Ids::ExprId id, std::ostringstream& out,
                                const ExprNameMap& names, const PrintOptions& options) {
    const Expr& e = (*store)[id];

    if (options.showExprIds)
        out << "#" << id.value() << ":";

    switch (e.op) {
    case OpType::Const:
        out << e.knownValue;

        if (options.showBitWidth)
            out << ":" << e.bitWidth;

        return;

    case OpType::Var: {
        auto it = names.find(id);

        if (it != names.end())
            out << it->second;
        else
            out << "v" << id.value();

        if (options.showBitWidth)
            out << ":" << e.bitWidth;

        return;
    }

    default:
        break;
    }

    out << OpTypeName(e.op) << "(";

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        if (i > 0)
            out << ", ";

        PrintDebugStructure(store, e.inputs[i], out, names, options);
    }

    out << ")";

    if (options.showBitWidth)
        out << ":" << e.bitWidth;
}

static void Print(const ExprStore* store, Ids::ExprId id, std::ostringstream& out, const ExprNameMap& names,
                  const PrintOptions& options, int parentPrecedence, bool isRightChild, OpType parentOp) {
    if (options.debugStructure) {
        PrintDebugStructure(store, id, out, names, options);
        return;
    }

    const Expr& e = (*store)[id];

    if (options.showExprIds)
        out << "#" << id.value() << ":";

    if (e.op == OpType::Const) {
        out << e.knownValue;

        if (options.showBitWidth)
            out << ":" << e.bitWidth;

        return;
    }

    if (options.showOpTypes && e.op != OpType::Var && e.op != OpType::Const) {

        out << OpTypeName(e.op) << "(";

        for (size_t i = 0; i < e.inputs.size(); ++i) {
            if (i > 0)
                out << ", ";

            Print(store, e.inputs[i], out, names, options, 0, false, OpType::Var);
        }

        out << ")";

        return;
    }

    if (e.op == OpType::Var) {
        auto it = names.find(id);

        if (it != names.end())
            out << it->second;
        else
            out << "v" << id.value();

        if (options.showBitWidth)
            out << ":" << e.bitWidth;

        return;
    }

    if (e.op == OpType::Not || e.op == OpType::Neg) {
        out << (e.op == OpType::Not ? "~" : "-");

        Ids::ExprId inner = e.inputs[0];

        const bool needsParens = options.explicitGroups || (PrecedenceOf(store, inner) < PrecedenceOf(store, id));

        if (needsParens) {
            out << "(";
            Print(store, inner, out, names, options, 0, false, OpType::Var);
            out << ")";
        } else {
            Print(store, inner, out, names, options, PrecedenceOf(store, id), true, e.op);
        }

        if (options.showBitWidth)
            out << ":" << e.bitWidth;

        return;
    }

    if (e.op == OpType::Pow) {
        if (options.powAsFunction) {
            out << "pow(";

            Print(store, e.inputs[0], out, names, options, 0, false, OpType::Var);

            out << ", ";

            Print(store, e.inputs[1], out, names, options, 0, true, OpType::Var);

            out << ")";

            if (options.showBitWidth)
                out << ":" << e.bitWidth;

            return;
        }
    }

    const OpInfo* info = GetOpInfo(e.op);

    const int currentPrecedence = info->precedence;

    bool wrapSelf = options.explicitGroups;

    if (!wrapSelf) {
        if (isRightChild && (parentOp == OpType::Shl || parentOp == OpType::Shr) &&
            (e.op == OpType::Add || e.op == OpType::Sub))
            wrapSelf = true;
        else if (currentPrecedence < parentPrecedence)
            wrapSelf = true;
        else if (isRightChild && currentPrecedence == parentPrecedence)
            wrapSelf = NeedsParensForRightChild(parentOp, e.op);
    }

    if (wrapSelf)
        out << "(";

    for (std::size_t i = 0; i < e.inputs.size(); ++i) {
        if (i > 0)
            out << " " << info->symbol << " ";

        Print(store, e.inputs[i], out, names, options, currentPrecedence, i > 0, e.op);
    }

    if (wrapSelf)
        out << ")";

    if (options.showBitWidth)
        out << ":" << e.bitWidth;
}

std::string ToString(const ExprStore* store, const Ids::ExprId e) {
    std::ostringstream out;
    Print(store, e, out, {}, PrintOptions{}, 0, false, OpType::Var);
    return out.str();
}

std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names) {
    std::ostringstream out;
    Print(store, e, out, names, PrintOptions{}, 0, false, OpType::Var);
    return out.str();
}

std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names,
                     const PrintOptions& options) {
    std::ostringstream out;
    Print(store, e, out, names, options, 0, false, OpType::Var);
    return out.str();
}

} // namespace BitFlow::Core::Expression