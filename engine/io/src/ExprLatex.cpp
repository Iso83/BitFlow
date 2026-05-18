#include <BitFlow/core/expression/Expr.h>
#include <BitFlow/core/expression/OpInfo.h>
#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/helper/Exception.h>
#include <sstream>

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
        return "\\frac";

    case OpType::Mod:
        return "\\bmod";

    case OpType::And:
        return "\\land";

    case OpType::Or:
        return "\\lor";

    case OpType::Xor:
        return "\\oplus";

    case OpType::Shl:
        return "\\ll";

    case OpType::Shr:
        return "\\gg";

    case OpType::Not:
        return "\\sim";

    default:
        BF_IO_THROW("Missing latex operator");
    }
}

static bool NeedsParens(const ExprStore* store, ExprId parent, ExprId child, bool isRightChild) {
    const Expr& p = (*store)[parent];
    const Expr& c = (*store)[child];

    if ((p.op == OpType::Shl || p.op == OpType::Shr) && (c.op == OpType::Add || c.op == OpType::Sub))
        return true;

    if ((p.op == OpType::And || p.op == OpType::Or || p.op == OpType::Xor) &&
        (c.op == OpType::Add || c.op == OpType::Sub))
        return true;

    if ((p.op == OpType::Mul || p.op == OpType::Div) && c.op == OpType::Div)
        return false;

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

    case OpType::Pow: {
        const ExprId base = expr.inputs[0];
        const ExprId exponent = expr.inputs[1];

        const Expr& baseExpr = (*store)[base];

        if (baseExpr.op == OpType::Div) {
            out << "\\left(";
            WriteLatex(out, store, base, names);
            out << "\\right)";
        } else {
            const bool baseParens = NeedsParens(store, id, base, false);

            if (baseParens)
                out << "(";

            WriteLatex(out, store, base, names);

            if (baseParens)
                out << ")";
        }

        out << "^{";
        WriteLatex(out, store, exponent, names);
        out << "}";

        return;
    }

    default:
        break;
    }

    if (expr.inputs.empty())
        BF_IO_THROW("Unsupported expression shape in ToLatex");

    if (expr.op == OpType::Div) {
        if (expr.inputs.size() != 2)
            BF_IO_THROW("Unsupported expression shape in ToLatex");

        out << "\\frac{";
        WriteLatex(out, store, expr.inputs[0], names);
        out << "}{";
        WriteLatex(out, store, expr.inputs[1], names);
        out << "}";
        return;
    }

    for (std::size_t i = 0; i < expr.inputs.size(); ++i) {
        if (i > 0)
            out << " " << OpToLatex(expr.op) << " ";

        const ExprId input = expr.inputs[i];
        const bool parens = NeedsParens(store, id, input, i > 0);

        if (parens)
            out << "(";

        WriteLatex(out, store, input, names);

        if (parens)
            out << ")";
    }

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
