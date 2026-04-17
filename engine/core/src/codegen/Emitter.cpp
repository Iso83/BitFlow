#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

static constexpr const char* kUnsupportedExpr = "0ull /* unsupported */";
static constexpr const char* kDefaultType = "uint64_t";

static std::string MakeVarName(uint32_t id) {
    return "v" + std::to_string(id);
}

static std::string BitWidthLiteral(uint32_t bw) {
    return std::to_string(bw) + "ull";
}

static int GetPrecedence(OpType op) {
    switch (op) {
    case OpType::Const:
    case OpType::Var:
        return 80; // primary / leaf
    case OpType::Not:
    case OpType::Neg:
        return 70; // unary
    case OpType::Mul:
    case OpType::Div:
    case OpType::Mod:
        return 60;
    case OpType::Add:
    case OpType::Sub:
        return 50;
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR:
        return 40;
    case OpType::And:
        return 30;
    case OpType::Xor:
        return 20;
    case OpType::Or:
        return 10;
    default:
        return 0;
    }
}

static bool NeedsParens(OpType parentOp, const Expr* child, bool isRightChild) {
    if (!child)
        return true;

    const int parentPrec = GetPrecedence(parentOp);
    const int childPrec = GetPrecedence(child->op);

    if (childPrec < parentPrec)
        return true;

    if (childPrec > parentPrec)
        return false;

    if (!isRightChild)
        return false;

    switch (parentOp) {
    case OpType::Add:
    case OpType::Mul:
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
        return false;
    default:
        return true;
    }
}

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "0xffffffffffffffffull";

    return "((1ull << " + std::to_string(bw) + ") - 1ull)";
}

static std::string ApplyMask(const std::string& expr, uint32_t bw) {
    return "((" + expr + ") & " + MakeMask(bw) + ")";
}

static std::string NormalizeShift(const std::string& rhs, uint32_t bw) {
    return "((" + rhs + ") % " + BitWidthLiteral(bw) + ")";
}

static std::string MakeRotateExpr(const std::string& value, const std::string& shift, uint32_t bw, bool left) {
    const std::string bwLiteral = BitWidthLiteral(bw);
    const std::string mask = MakeMask(bw);

    if (left)
        return "(((" + value + " << " + shift + ") | (" + value + " >> (" + bwLiteral + " - " + shift + "))) & " +
               mask + ")";

    return "(((" + value + " >> " + shift + ") | (" + value + " << (" + bwLiteral + " - " + shift + "))) & " + mask +
           ")";
}

static bool IsWrapped(const std::string& text) {
    return text.size() >= 2 && text.front() == '(' && text.back() == ')';
}

static std::string EmitBinary(const std::string& lhs, const char* op, const std::string& rhs) {
    return lhs + " " + op + " " + rhs;
}

static std::string EmitUnary(const char* op, const std::string& value) {
    return std::string(op) + value;
}

static std::string MaybeWrapChild(const std::string& emittedChild, OpType parentOp, const Expr* child,
                                  bool isRightChild) {
    if (NeedsParens(parentOp, child, isRightChild) && !IsWrapped(emittedChild))
        return "(" + emittedChild + ")";
    return emittedChild;
}

static bool ShouldWrapForParent(OpType selfOp, int parentPrec, bool isRightChild) {
    if (parentPrec < 0)
        return false;

    const int selfPrec = GetPrecedence(selfOp);
    if (selfPrec < parentPrec)
        return true;

    if (isRightChild && selfPrec == parentPrec)
        return true;

    return false;
}

static std::string EmitNode(const Expr* e, uint32_t bw, int parentPrec = -1, bool isRightChild = false) {
    using enum OpType;

    if (e->op == OpType::Const) {
        std::string emitted = ApplyMask(std::to_string(e->constValue) + "ull", bw);
        if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
            return "(" + emitted + ")";
        return emitted;
    }

    if (e->op == OpType::Var) {
        std::string emitted = ApplyMask(MakeVarName(e->id.value()), bw);
        if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
            return "(" + emitted + ")";
        return emitted;
    }

    if (e->inputs.size() == 1) {
        const int currentPrec = GetPrecedence(e->op);
        std::string a = EmitNode(e->inputs[0], bw, currentPrec, true);
        a = MaybeWrapChild(a, e->op, e->inputs[0], true);
        std::string emitted;

        switch (e->op) {
        case Neg:
            emitted = ApplyMask(EmitUnary("-", a), bw);
            break;
        case Not:
            emitted = ApplyMask(EmitUnary("~", a), bw);
            break;
        default:
            break;
        }

        if (!emitted.empty()) {
            if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
                return "(" + emitted + ")";
            return emitted;
        }
    }

    if (e->inputs.size() >= 2) {
        const int currentPrec = GetPrecedence(e->op);
        std::string lhs = EmitNode(e->inputs[0], bw, currentPrec, false);

        for (size_t i = 1; i < e->inputs.size(); ++i) {
            std::string rhs = EmitNode(e->inputs[i], bw, currentPrec, true);
            std::string sh = NormalizeShift(rhs, bw);
            const Expr* leftExpr = (i == 1) ? e->inputs[0] : nullptr;
            const std::string lhsWrapped = MaybeWrapChild(lhs, e->op, leftExpr, false);
            const std::string rhsWrapped = MaybeWrapChild(rhs, e->op, e->inputs[i], true);

            switch (e->op) {
            case Add:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "+", rhsWrapped), bw);
                break;
            case Sub:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "-", rhsWrapped), bw);
                break;
            case Mul:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "*", rhsWrapped), bw);
                break;
            case Div:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "/", rhsWrapped), bw);
                break;
            case Mod:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "%", rhsWrapped), bw);
                break;

            case And:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "&", rhsWrapped), bw);
                break;
            case Or:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "|", rhsWrapped), bw);
                break;
            case Xor:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "^", rhsWrapped), bw);
                break;

            case Shl:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "<<", sh), bw);
                break;
            case Shr:
            case UShr:
                lhs = ApplyMask(EmitBinary(lhsWrapped, ">>", sh), bw);
                break;

            case RotL: {
                lhs = MakeRotateExpr(lhs, sh, bw, true);
                break;
            }
            case RotR: {
                lhs = MakeRotateExpr(lhs, sh, bw, false);
                break;
            }
            default:
                return "";
            }
        }

        if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
            return "(" + lhs + ")";
        return lhs;
    }

    return "";
}

static void CollectVars(const Expr* e, std::set<uint32_t>& out) {
    if (!e)
        return;

    if (e->op == OpType::Var)
        out.insert(e->id.value());

    for (const Expr* input : e->inputs)
        CollectVars(input, out);
}

static void CollectVarsMulti(const std::vector<const Expr*>& roots, std::set<uint32_t>& out) {
    for (const Expr* root : roots)
        CollectVars(root, out);
}

static void CollectUseCount(const Expr* e, std::unordered_map<const Expr*, size_t>& useCount,
                            std::unordered_set<const Expr*>& visited) {
    if (!e)
        return;
    useCount[e] += 1;
    if (!visited.insert(e).second)
        return;
    for (const Expr* input : e->inputs)
        CollectUseCount(input, useCount, visited);
}

static std::unordered_map<const Expr*, size_t> BuildUseCountMap(const std::vector<const Expr*>& roots) {
    std::unordered_map<const Expr*, size_t> useCount;
    std::unordered_set<const Expr*> visited;
    for (const Expr* root : roots)
        CollectUseCount(root, useCount, visited);
    return useCount;
}

static bool ShouldMaterializeNode(const Expr* e, const std::unordered_map<const Expr*, size_t>& useCount) {
    if (!e || e->op == OpType::Const || e->op == OpType::Var)
        return false;
    auto it = useCount.find(e);
    if (it == useCount.end())
        return false;
    return it->second > 1;
}

static bool IsIdentifierStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool IsIdentifierChar(char c) {
    return IsIdentifierStart(c) || (c >= '0' && c <= '9');
}

static bool IsValidIdentifier(const std::string& name) {
    if (name.empty())
        return false;

    if (!IsIdentifierStart(name.front()))
        return false;

    return std::all_of(name.begin() + 1, name.end(), IsIdentifierChar);
}

static bool IsWordChar(char c) {
    return IsIdentifierChar(c);
}

static void ReplaceIdentifierToken(std::string& text, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        const bool leftBoundary = (pos == 0) || !IsWordChar(text[pos - 1]);
        const size_t rightPos = pos + from.size();
        const bool rightBoundary = (rightPos >= text.size()) || !IsWordChar(text[rightPos]);
        if (leftBoundary && rightBoundary) {
            text.replace(pos, from.size(), to);
            pos += to.size();
        } else {
            pos += from.size();
        }
    }
}

} // namespace

std::string EmitCExpr(const Expr* root, uint32_t bitWidth) {
    if (!root)
        return kUnsupportedExpr;

    std::string expr = EmitNode(root, bitWidth, -1, false);
    if (expr.empty())
        return kUnsupportedExpr;

    return ApplyMask(expr, bitWidth);
}

std::string EmitCFunction(const Expr* root, uint32_t bitWidth) {
    std::set<uint32_t> vars;
    CollectVars(root, vars);

    std::vector<uint32_t> ordered(vars.begin(), vars.end());

    std::ostringstream out;

    out << "uint64_t f(";

    for (size_t i = 0; i < ordered.size(); ++i) {
        if (i > 0)
            out << ", ";
        out << "uint64_t v" << ordered[i];
    }

    out << ") {\n";
    out << "    return " << EmitCExpr(root, bitWidth) << ";\n";
    out << "}";

    return out.str();
}

std::map<uint32_t, std::string> BuildVarNameMap(const Expr* root, const std::map<uint32_t, std::string>& overrides) {
    std::set<uint32_t> sorted;
    CollectVars(root, sorted);
    std::vector<uint32_t> vars(sorted.begin(), sorted.end());

    std::map<uint32_t, std::string> result;
    for (const uint32_t id : vars) {
        auto it = overrides.find(id);
        if (it != overrides.end() && IsValidIdentifier(it->second)) {
            result[id] = it->second;
            continue;
        }
        result[id] = MakeVarName(id);
    }
    return result;
}

std::string EmitCParamList(const Expr* root, uint32_t bitWidth, const std::map<uint32_t, std::string>& varNames) {
    (void)bitWidth;
    std::map<uint32_t, std::string> resolvedNames = BuildVarNameMap(root, varNames);
    std::string params;
    bool first = true;
    for (const auto& [id, name] : resolvedNames) {
        (void)id;
        if (!first)
            params += ", ";
        params += std::string(kDefaultType) + " " + name;
        first = false;
    }
    return params;
}

std::string EmitCFunction(const Expr* root, uint32_t bitWidth, const std::string& functionName,
                          const std::map<uint32_t, std::string>& varNames) {
    std::map<uint32_t, std::string> resolvedNames = BuildVarNameMap(root, varNames);

    std::string expr = EmitCExpr(root, bitWidth);
    for (const auto& [id, name] : resolvedNames) {
        const std::string fallback = MakeVarName(id);
        ReplaceIdentifierToken(expr, fallback, name);
    }

    std::string out;
    out += std::string(kDefaultType) + " " + functionName + "(";
    out += EmitCParamList(root, bitWidth, varNames);
    out += ") {\n";
    out += "    return " + expr + ";\n";
    out += "}";
    return out;
}

std::string EmitCFunctionMulti(const std::vector<const Expr*>& outputs, uint32_t bitWidth) {
    return EmitCFunction(outputs, bitWidth);
}

std::string EmitCFunction(const std::vector<const Expr*>& roots, uint32_t bitWidth) {
    return EmitCFunction(roots, bitWidth, "f", {});
}

std::string EmitCFunction(const std::vector<const Expr*>& roots, uint32_t bitWidth, const std::string& functionName,
                          const std::map<uint32_t, std::string>& varNames) {
    if (roots.empty()) {
        return std::string("void ") + functionName + "() {\n}";
    }

    std::set<uint32_t> sortedVars;
    CollectVarsMulti(roots, sortedVars);
    std::vector<uint32_t> vars(sortedVars.begin(), sortedVars.end());
    const std::unordered_map<const Expr*, size_t> useCount = BuildUseCountMap(roots);

    std::map<uint32_t, std::string> resolvedNames;
    for (const uint32_t id : vars) {
        auto it = varNames.find(id);
        if (it != varNames.end() && IsValidIdentifier(it->second)) {
            resolvedNames[id] = it->second;
            continue;
        }
        resolvedNames[id] = MakeVarName(id);
    }

    std::vector<std::string> tempDecls;
    std::unordered_map<const Expr*, std::string> tempByExpr;
    size_t tempIndex = 1;

    std::function<std::string(const Expr*, int, bool)> emitWithTemps;
    emitWithTemps = [&](const Expr* e, int parentPrec, bool isRightChild) -> std::string {
        if (!e)
            return kUnsupportedExpr;

        if (ShouldMaterializeNode(e, useCount)) {
            auto itTemp = tempByExpr.find(e);
            if (itTemp == tempByExpr.end()) {
                const std::string tempName = "t" + std::to_string(tempIndex++);
                tempByExpr[e] = tempName;
                const std::string rhs = EmitNode(e, bitWidth, -1, false);
                std::string rhsNamed = rhs;
                for (const auto& [id, name] : resolvedNames)
                    ReplaceIdentifierToken(rhsNamed, MakeVarName(id), name);
                for (const auto& [exprNode, generatedTemp] : tempByExpr) {
                    (void)exprNode;
                    ReplaceIdentifierToken(rhsNamed, ApplyMask(generatedTemp, bitWidth), generatedTemp);
                }
                tempDecls.push_back("    " + std::string(kDefaultType) + " " + tempName + " = " + rhsNamed + ";");
                itTemp = tempByExpr.find(e);
            }
            std::string emitted = itTemp->second;
            if (ShouldWrapForParent(OpType::Var, parentPrec, isRightChild))
                emitted = "(" + emitted + ")";
            return emitted;
        }

        if (e->op == OpType::Const) {
            std::string emitted = ApplyMask(std::to_string(e->constValue) + "ull", bitWidth);
            if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
                return "(" + emitted + ")";
            return emitted;
        }

        if (e->op == OpType::Var) {
            std::string emitted = ApplyMask(MakeVarName(e->id.value()), bitWidth);
            if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
                return "(" + emitted + ")";
            return emitted;
        }

        if (e->inputs.size() == 1) {
            const int currentPrec = GetPrecedence(e->op);
            std::string a = emitWithTemps(e->inputs[0], currentPrec, true);
            a = MaybeWrapChild(a, e->op, e->inputs[0], true);
            std::string emitted;

            switch (e->op) {
            case OpType::Neg:
                emitted = ApplyMask(EmitUnary("-", a), bitWidth);
                break;
            case OpType::Not:
                emitted = ApplyMask(EmitUnary("~", a), bitWidth);
                break;
            default:
                break;
            }

            if (!emitted.empty()) {
                if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
                    return "(" + emitted + ")";
                return emitted;
            }
        }

        if (e->inputs.size() >= 2) {
            const int currentPrec = GetPrecedence(e->op);
            std::string lhs = emitWithTemps(e->inputs[0], currentPrec, false);

            for (size_t i = 1; i < e->inputs.size(); ++i) {
                std::string rhs = emitWithTemps(e->inputs[i], currentPrec, true);
                std::string sh = NormalizeShift(rhs, bitWidth);
                const Expr* leftExpr = (i == 1) ? e->inputs[0] : nullptr;
                const std::string lhsWrapped = MaybeWrapChild(lhs, e->op, leftExpr, false);
                const std::string rhsWrapped = MaybeWrapChild(rhs, e->op, e->inputs[i], true);

                switch (e->op) {
                case OpType::Add:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "+", rhsWrapped), bitWidth);
                    break;
                case OpType::Sub:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "-", rhsWrapped), bitWidth);
                    break;
                case OpType::Mul:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "*", rhsWrapped), bitWidth);
                    break;
                case OpType::Div:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "/", rhsWrapped), bitWidth);
                    break;
                case OpType::Mod:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "%", rhsWrapped), bitWidth);
                    break;
                case OpType::And:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "&", rhsWrapped), bitWidth);
                    break;
                case OpType::Or:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "|", rhsWrapped), bitWidth);
                    break;
                case OpType::Xor:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "^", rhsWrapped), bitWidth);
                    break;
                case OpType::Shl:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, "<<", sh), bitWidth);
                    break;
                case OpType::Shr:
                case OpType::UShr:
                    lhs = ApplyMask(EmitBinary(lhsWrapped, ">>", sh), bitWidth);
                    break;
                case OpType::RotL:
                    lhs = MakeRotateExpr(lhs, sh, bitWidth, true);
                    break;
                case OpType::RotR:
                    lhs = MakeRotateExpr(lhs, sh, bitWidth, false);
                    break;
                default:
                    return "";
                }
            }

            if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
                return "(" + lhs + ")";
            return lhs;
        }

        return "";
    };

    std::string out;
    out += "void " + functionName + "(";

    bool first = true;
    for (const auto& [id, name] : resolvedNames) {
        (void)id;
        if (!first)
            out += ", ";
        out += std::string(kDefaultType) + " " + name;
        first = false;
    }
    for (size_t i = 0; i < roots.size(); ++i) {
        if (!first)
            out += ", ";
        out += std::string(kDefaultType) + "& out" + std::to_string(i);
        first = false;
    }
    out += ") {\n";

    std::vector<std::string> outputExprs;
    outputExprs.reserve(roots.size());
    for (const Expr* root : roots) {
        std::string expr = emitWithTemps(root, -1, false);
        if (expr.empty())
            expr = kUnsupportedExpr;
        outputExprs.push_back(ApplyMask(expr, bitWidth));
    }

    for (const std::string& decl : tempDecls)
        out += decl + "\n";
    for (size_t i = 0; i < outputExprs.size(); ++i)
        out += "    out" + std::to_string(i) + " = " + outputExprs[i] + ";\n";

    out += "}";
    return out;
}

} // namespace BitFlow::Core::Codegen
