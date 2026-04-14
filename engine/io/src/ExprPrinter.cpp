#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/io/ExprPrinter.h>
#include <sstream>

namespace BitFlow::IO {

using Expr = Core::AST::Expr;
using OpType = Core::AST::OpType;

static void Print(const Expr* e, std::ostringstream& out, const std::unordered_map<uint32_t, std::string>& names) {
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

    if (e->op == OpType::Not) {
        out << "~(";
        Print(e->inputs[0], out, names);
        out << ")";
        return;
    }

    const char* op = "?";
    switch (e->op) {
    case OpType::And:
        op = "&";
        break;
    case OpType::Or:
        op = "|";
        break;
    case OpType::Xor:
        op = "^";
        break;
    default:
        break;
    }

    out << "(";
    for (size_t i = 0; i < e->inputs.size(); ++i) {
        if (i > 0)
            out << " " << op << " ";

        Print(e->inputs[i], out, names);
    }
    out << ")";
}

std::string ToString(const Expr* e) {
    std::ostringstream out;
    Print(e, out, {});
    return out.str();
}

std::string ToString(const Expr* e, const std::unordered_map<uint32_t, std::string>& names) {
    std::ostringstream out;
    Print(e, out, names);
    return out.str();
}

} // namespace BitFlow::IO