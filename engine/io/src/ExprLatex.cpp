#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/OpInfo.h>
#include <BitFlow/io/ExprLatex.h>
#include <sstream>
#include <stdexcept>

namespace BitFlow::IO {

using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

static std::string OpToLatex(OpType op) {
    switch (op) {
    case OpType::Add:
        return "+";

    case OpType::Sub:
        return "-";

    case OpType::Mul:
        return "\\cdot";

    case OpType::Div:
        return "/";

    case OpType::Mod:
        return "\\bmod";

    case OpType::And:
        return "\\mathbin{\\&}";

    case OpType::Or:
        return "|";

    case OpType::Xor:
        return "\\oplus";

    case OpType::Shl:
        return "\\ll";

    case OpType::Shr:
        return "\\gg";

    case OpType::Not:
        return "\\sim";

    default:
        return "?";
    }
}

static bool NeedsParens(const ExprStore* store, ExprId parent, ExprId child, bool isRightChild) {
    const Expr& p = (*store)[parent];
    const Expr& c = (*store)[child];

    if ((p.op == OpType::Shl || p.op == OpType::Shr) && (c.op == OpType::Add || c.op == OpType::Sub))
        return true;

    return RequiresParentheses(p.op, c.op, isRightChild);
}

static void WriteLatex(std::ostringstream& out, const ExprStore* store, ExprId id, const ExprNameMap* names) {

    const Expr& expr = (*store)[id];

    switch (expr.op) {
    case OpType::Const:
        out << expr.knownValue;
        return;

    case OpType::Var:
        if (names) {
            auto it = names->find(id);

            if (it != names->end()) {
                out << it->second;
                return;
            }
        }

        out << "v" << id.value();
        return;

    case OpType::Not: {
        out << OpToLatex(expr.op) << " ";

        const ExprId inner = expr.inputs[0];

        const bool parens = NeedsParens(store, id, inner, false);

        if (parens)
            out << "(";

        WriteLatex(out, store, inner, names);

        if (parens)
            out << ")";

        return;
    }

    case OpType::Neg: {
        out << "-";

        const ExprId inner = expr.inputs[0];

        const bool parens = NeedsParens(store, id, inner, false);

        if (parens)
            out << "(";

        WriteLatex(out, store, inner, names);

        if (parens)
            out << ")";

        return;
    }

    case OpType::RotL:
    case OpType::RotR: {
        out << "\\operatorname{";

        if (expr.op == OpType::RotL)
            out << "rotl";
        else
            out << "rotr";

        out << "}(";

        WriteLatex(out, store, expr.inputs[0], names);

        out << ", ";

        WriteLatex(out, store, expr.inputs[1], names);

        out << ")";

        return;
    }

    default:
        break;
    }

    if (expr.inputs.size() != 2)
        throw std::runtime_error("Unsupported expression shape in ToLatex");

    const ExprId left = expr.inputs[0];
    const ExprId right = expr.inputs[1];

    const bool leftParens = NeedsParens(store, id, left, false);

    const bool rightParens = NeedsParens(store, id, right, true);

    if (leftParens)
        out << "(";

    WriteLatex(out, store, left, names);

    if (leftParens)
        out << ")";

    out << " " << OpToLatex(expr.op) << " ";

    if (rightParens)
        out << "(";

    WriteLatex(out, store, right, names);

    if (rightParens)
        out << ")";

    return;
}

std::string ToLatex(const ExprStore* store, ExprId root, const ExprNameMap& names) {

    std::ostringstream out;

    WriteLatex(out, store, root, &names);

    return out.str();
}

std::string ToLatex(ExprRef root, const ExprNameMap& names) {

    return ToLatex(root.store, root.id, names);
}

} // namespace BitFlow::IO