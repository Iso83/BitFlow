#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen/TypeMap.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

static constexpr const char* kUnsupportedExpr = "0ull /* unsupported */";

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
                                  bool isRightChild);

static std::string CombineNode(const Expr* e, uint32_t bw, const std::vector<std::string>& childExprs) {
    if (!e)
        return "";

    if (e->op == OpType::Const)
        return ApplyMask(std::to_string(e->constValue) + "ull", bw);

    if (e->op == OpType::Var)
        return ApplyMask(MakeVarName(e->id.value()), bw);

    if (e->inputs.size() == 1 && childExprs.size() == 1) {
        std::string a = MaybeWrapChild(childExprs[0], e->op, e->inputs[0], true);
        switch (e->op) {
        case OpType::Neg:
            return ApplyMask(EmitUnary("-", a), bw);
        case OpType::Not:
            return ApplyMask(EmitUnary("~", a), bw);
        default:
            return "";
        }
    }

    if (e->inputs.size() >= 2 && childExprs.size() == e->inputs.size()) {
        std::string lhs = childExprs[0];
        for (size_t i = 1; i < childExprs.size(); ++i) {
            std::string rhs = childExprs[i];
            std::string sh = NormalizeShift(rhs, bw);
            const Expr* leftExpr = (i == 1) ? e->inputs[0] : nullptr;
            const std::string lhsWrapped = MaybeWrapChild(lhs, e->op, leftExpr, false);
            const std::string rhsWrapped = MaybeWrapChild(rhs, e->op, e->inputs[i], true);

            switch (e->op) {
            case OpType::Add:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "+", rhsWrapped), bw);
                break;
            case OpType::Sub:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "-", rhsWrapped), bw);
                break;
            case OpType::Mul:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "*", rhsWrapped), bw);
                break;
            case OpType::Div:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "/", rhsWrapped), bw);
                break;
            case OpType::Mod:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "%", rhsWrapped), bw);
                break;
            case OpType::And:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "&", rhsWrapped), bw);
                break;
            case OpType::Or:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "|", rhsWrapped), bw);
                break;
            case OpType::Xor:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "^", rhsWrapped), bw);
                break;
            case OpType::Shl:
                lhs = ApplyMask(EmitBinary(lhsWrapped, "<<", sh), bw);
                break;
            case OpType::Shr:
            case OpType::UShr:
                lhs = ApplyMask(EmitBinary(lhsWrapped, ">>", sh), bw);
                break;
            case OpType::RotL:
                lhs = MakeRotateExpr(lhs, sh, bw, true);
                break;
            case OpType::RotR:
                lhs = MakeRotateExpr(lhs, sh, bw, false);
                break;
            default:
                return "";
            }
        }
        return lhs;
    }

    return "";
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
    std::vector<std::string> childExprs;
    childExprs.reserve(e->inputs.size());
    const int currentPrec = GetPrecedence(e->op);
    for (size_t i = 0; i < e->inputs.size(); ++i)
        childExprs.push_back(EmitNode(e->inputs[i], bw, currentPrec, i > 0));

    std::string emitted = CombineNode(e, bw, childExprs);
    if (emitted.empty())
        return "";
    if (ShouldWrapForParent(e->op, parentPrec, isRightChild))
        return "(" + emitted + ")";
    return emitted;
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

static const char* OpTypeLabel(OpType op) {
    switch (op) {
    case OpType::Const:
        return "Const";
    case OpType::Var:
        return "Var";
    case OpType::Not:
        return "Not";
    case OpType::Neg:
        return "Neg";
    case OpType::Mul:
        return "Mul";
    case OpType::Div:
        return "Div";
    case OpType::Mod:
        return "Mod";
    case OpType::Add:
        return "Add";
    case OpType::Sub:
        return "Sub";
    case OpType::Shl:
        return "Shl";
    case OpType::Shr:
        return "Shr";
    case OpType::UShr:
        return "UShr";
    case OpType::RotL:
        return "RotL";
    case OpType::RotR:
        return "RotR";
    case OpType::And:
        return "And";
    case OpType::Xor:
        return "Xor";
    case OpType::Or:
        return "Or";
    default:
        return "Unknown";
    }
}

static std::string BuildStructuralKeyImpl(const Expr* e, std::unordered_map<const Expr*, std::string>& memo) {
    if (!e)
        return "N";

    auto it = memo.find(e);
    if (it != memo.end())
        return it->second;

    std::string key;
    if (e->op == OpType::Const) {
        key = "C(" + std::to_string(e->constValue) + ")";
    } else if (e->op == OpType::Var) {
        key = "V(" + std::to_string(e->id.value()) + ")";
    } else {
        // Exact structureel: input-volgorde en boom-vorm tellen mee.
        // Dus géén commutativiteit/associativiteit in deze sleutel:
        //  - (a ^ b) != (b ^ a)
        //  - a + (b + c) != (a + b) + c
        key = std::string(OpTypeLabel(e->op)) + "(";
        for (size_t i = 0; i < e->inputs.size(); ++i) {
            if (i > 0)
                key += ",";
            key += BuildStructuralKeyImpl(e->inputs[i], memo);
        }
        key += ")";
    }

    memo.emplace(e, key);
    return key;
}

static std::string BuildStructuralKey(const Expr* e) {
    std::unordered_map<const Expr*, std::string> memo;
    return BuildStructuralKeyImpl(e, memo);
}

static void CountStructuralUses(const Expr* e, std::unordered_map<std::string, uint32_t>& counts) {
    if (!e)
        return;
    counts[BuildStructuralKey(e)] += 1;
    for (const Expr* input : e->inputs)
        CountStructuralUses(input, counts);
}

static void CollectStructuralRepresentatives(const Expr* e, std::unordered_map<std::string, const Expr*>& repsByKey) {
    if (!e)
        return;

    const std::string key = BuildStructuralKey(e);
    if (repsByKey.find(key) == repsByKey.end())
        repsByKey.emplace(key, e); // first encountered node is representative

    for (const Expr* input : e->inputs)
        CollectStructuralRepresentatives(input, repsByKey);
}

static std::unordered_map<std::string, uint32_t>
BuildUseCountMap(const std::vector<const Expr*>& roots, std::unordered_map<const Expr*, std::string>& keyMemo) {
    std::unordered_map<std::string, uint32_t> useCount;
    for (const Expr* root : roots) {
        BuildStructuralKeyImpl(root, keyMemo);
        CountStructuralUses(root, useCount);
    }
    return useCount;
}

static std::unordered_map<std::string, const Expr*> BuildRepresentativeMap(const std::vector<const Expr*>& roots) {
    std::unordered_map<std::string, const Expr*> repsByKey;
    for (const Expr* root : roots)
        CollectStructuralRepresentatives(root, repsByKey);
    return repsByKey;
}

static bool IsTempEligible(const Expr* e, uint32_t structuralUseCount) {
    if (!e)
        return false;
    return structuralUseCount > 1 && e->op != OpType::Const && e->op != OpType::Var;
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

struct TempEmitState {
    uint32_t nextTempId = 1;
};

static std::string EmitNodeWithTemps(const Expr* e, uint32_t bw,
                                     const std::unordered_map<std::string, uint32_t>& counts,
                                     const std::unordered_map<const Expr*, std::string>& structuralKeys,
                                     const std::unordered_map<std::string, const Expr*>& repsByKey,
                                     std::unordered_map<std::string, std::string>& tempNamesByKey,
                                     std::vector<std::string>& statements, TempEmitState& tempState) {
    std::function<std::string(const Expr*, int, bool)> emitRec;
    emitRec = [&](const Expr* node, int parentPrec, bool isRightChild) -> std::string {
        if (!node)
            return kUnsupportedExpr;

        if (node->op == OpType::Const) {
            std::string emitted = ApplyMask(std::to_string(node->constValue) + "ull", bw);
            if (ShouldWrapForParent(node->op, parentPrec, isRightChild))
                return "(" + emitted + ")";
            return emitted;
        }

        if (node->op == OpType::Var) {
            std::string emitted = ApplyMask(MakeVarName(node->id.value()), bw);
            if (ShouldWrapForParent(node->op, parentPrec, isRightChild))
                return "(" + emitted + ")";
            return emitted;
        }

        const auto itNodeKey = structuralKeys.find(node);
        const std::string nodeKey = (itNodeKey != structuralKeys.end()) ? itNodeKey->second : std::string{};
        if (!nodeKey.empty()) {
            auto itAssigned = tempNamesByKey.find(nodeKey);
            if (itAssigned != tempNamesByKey.end()) {
                std::string emitted = itAssigned->second;
                if (ShouldWrapForParent(OpType::Var, parentPrec, isRightChild))
                    emitted = "(" + emitted + ")";
                return emitted;
            }
        }

        std::vector<std::string> childExprs;
        childExprs.reserve(node->inputs.size());
        const int currentPrec = GetPrecedence(node->op);
        for (size_t i = 0; i < node->inputs.size(); ++i)
            childExprs.push_back(emitRec(node->inputs[i], currentPrec, i > 0));

        // Semantische emissie blijft volledig in CombineNode.
        // Structurele CSE bepaalt hierna alleen of dit resultaat in een temp gaat.
        std::string emitted = CombineNode(node, bw, childExprs);

        if (emitted.empty())
            emitted = kUnsupportedExpr;

        auto itCount = counts.find(nodeKey);
        const uint32_t structuralUseCount = (itCount != counts.end()) ? itCount->second : 0u;
        const Expr* representative = node;
        auto itRep = repsByKey.find(nodeKey);
        if (itRep != repsByKey.end())
            representative = itRep->second;

        if (IsTempEligible(node, structuralUseCount) && representative == node) {
            const std::string tempName = "t" + std::to_string(tempState.nextTempId++);
            tempNamesByKey[nodeKey] = tempName;
            statements.push_back("    " + GetCType(bw) + " " + tempName + " = " + ApplyMask(emitted, bw) + ";");
            std::string result = tempName;
            if (ShouldWrapForParent(OpType::Var, parentPrec, isRightChild))
                result = "(" + result + ")";
            return result;
        }

        if (ShouldWrapForParent(node->op, parentPrec, isRightChild))
            return "(" + emitted + ")";
        return emitted;
    };

    return emitRec(e, -1, false);
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
    std::map<uint32_t, std::string> resolvedNames = BuildVarNameMap(root, varNames);
    std::string params;
    bool first = true;
    for (const auto& [id, name] : resolvedNames) {
        (void)id;
        if (!first)
            params += ", ";
        params += GetCType(bitWidth) + " " + name;
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
    out += GetCType(bitWidth) + " " + functionName + "(";
    out += EmitCParamList(root, bitWidth, varNames);
    out += ") {\n";
    out += "    return " + expr + ";\n";
    out += "}";
    return out;
}

std::string EmitCFunctionMulti(const std::vector<const Expr*>& outputs, uint32_t bitWidth) {
    const std::string functionName = "f";
    const std::map<uint32_t, std::string> varNames = {};

    if (outputs.empty()) {
        std::string out;
        out += "struct Outputs {\n";
        out += "};\n\n";
        out += "Outputs " + functionName + "() {\n";
        out += "    Outputs r{};\n";
        out += "    return r;\n";
        out += "}";
        return out;
    }

    // 1) variabelen verzamelen uit alle outputs
    std::set<uint32_t> sortedVars;
    CollectVarsMulti(outputs, sortedVars);
    std::vector<uint32_t> vars(sortedVars.begin(), sortedVars.end());

    // 2) use-counts opbouwen over alle outputs
    std::unordered_map<const Expr*, std::string> structuralKeys;
    const std::unordered_map<std::string, uint32_t> useCountsByKey = BuildUseCountMap(outputs, structuralKeys);
    const std::unordered_map<std::string, const Expr*> repsByKey = BuildRepresentativeMap(outputs);

    // naam-resolutie voor parameters
    std::map<uint32_t, std::string> resolvedNames;
    for (const uint32_t id : vars) {
        auto it = varNames.find(id);
        if (it != varNames.end() && IsValidIdentifier(it->second)) {
            resolvedNames[id] = it->second;
            continue;
        }
        resolvedNames[id] = MakeVarName(id);
    }

    // 3) shared temps genereren
    std::vector<std::string> statements;
    std::unordered_map<std::string, std::string> tempNamesByKey;
    TempEmitState tempState{};

    // 4) per output outN toekennen
    std::vector<std::string> outputExprs;
    outputExprs.reserve(outputs.size());
    for (const Expr* root : outputs) {
        std::string expr = EmitNodeWithTemps(root, bitWidth, useCountsByKey, structuralKeys, repsByKey, tempNamesByKey,
                                             statements, tempState);
        if (expr.empty())
            expr = kUnsupportedExpr;
        outputExprs.push_back(ApplyMask(expr, bitWidth));
    }

    for (const auto& [id, name] : resolvedNames) {
        const std::string fallback = MakeVarName(id);
        for (std::string& stmt : statements)
            ReplaceIdentifierToken(stmt, fallback, name);
        for (std::string& expr : outputExprs)
            ReplaceIdentifierToken(expr, fallback, name);
    }

    // 5) code als functie teruggeven
    std::string out;
    out += "struct Outputs {\n";
    for (size_t i = 0; i < outputs.size(); ++i)
        out += "    " + GetCType(bitWidth) + " out" + std::to_string(i + 1) + ";\n";
    out += "};\n\n";
    out += "Outputs " + functionName + "(";
    bool first = true;
    for (const auto& [id, name] : resolvedNames) {
        (void)id;
        if (!first)
            out += ", ";
        out += GetCType(bitWidth) + " " + name;
        first = false;
    }
    out += ") {\n";
    out += "    Outputs r{};\n";
    for (const std::string& stmt : statements)
        out += stmt + "\n";
    for (size_t i = 0; i < outputExprs.size(); ++i)
        out += "    r.out" + std::to_string(i + 1) + " = " + outputExprs[i] + ";\n";
    out += "    return r;\n";
    out += "}";
    return out;
}

std::string EmitCFunction(const std::vector<const Expr*>& roots, uint32_t bitWidth) {
    return EmitCFunctionMulti(roots, bitWidth);
}

std::string EmitCFunction(const std::vector<const Expr*>& roots, uint32_t bitWidth, const std::string& functionName,
                          const std::map<uint32_t, std::string>& varNames) {
    if (roots.empty()) {
        std::string out;
        out += "struct Outputs {\n";
        out += "};\n\n";
        out += "Outputs " + functionName + "() {\n";
        out += "    Outputs r{};\n";
        out += "    return r;\n";
        out += "}";
        return out;
    }

    std::set<uint32_t> sortedVars;
    CollectVarsMulti(roots, sortedVars);
    std::vector<uint32_t> vars(sortedVars.begin(), sortedVars.end());
    std::unordered_map<const Expr*, std::string> structuralKeys;
    const std::unordered_map<std::string, uint32_t> useCount = BuildUseCountMap(roots, structuralKeys);
    const std::unordered_map<std::string, const Expr*> repsByKey = BuildRepresentativeMap(roots);

    std::map<uint32_t, std::string> resolvedNames;
    for (const uint32_t id : vars) {
        auto it = varNames.find(id);
        if (it != varNames.end() && IsValidIdentifier(it->second))
            resolvedNames[id] = it->second;
        else
            resolvedNames[id] = MakeVarName(id);
    }
    std::string out;
    out += "struct Outputs {\n";
    for (size_t i = 0; i < roots.size(); ++i)
        out += "    " + GetCType(bitWidth) + " out" + std::to_string(i + 1) + ";\n";
    out += "};\n\n";
    out += "Outputs " + functionName + "(";
    bool first = true;
    for (const auto& [id, name] : resolvedNames) {
        (void)id;
        if (!first)
            out += ", ";
        out += GetCType(bitWidth) + " " + name;
        first = false;
    }
    out += ") {\n";
    out += "    Outputs r{};\n";

    std::vector<std::string> tempDecls;
    std::unordered_map<std::string, std::string> assignedNamesByKey;
    TempEmitState tempState{};
    std::vector<std::string> outputExprs;
    for (const Expr* root : roots) {
        std::string expr = EmitNodeWithTemps(root, bitWidth, useCount, structuralKeys, repsByKey, assignedNamesByKey,
                                             tempDecls, tempState);
        outputExprs.push_back(ApplyMask(expr.empty() ? std::string(kUnsupportedExpr) : expr, bitWidth));
    }
    for (const auto& [id, name] : resolvedNames) {
        const std::string fallback = MakeVarName(id);
        for (std::string& decl : tempDecls)
            ReplaceIdentifierToken(decl, fallback, name);
        for (std::string& expr : outputExprs)
            ReplaceIdentifierToken(expr, fallback, name);
    }
    for (const std::string& decl : tempDecls)
        out += decl + "\n";
    for (size_t i = 0; i < outputExprs.size(); ++i)
        out += "    r.out" + std::to_string(i + 1) + " = " + outputExprs[i] + ";\n";
    out += "    return r;\n";
    out += "}";
    return out;
}

} // namespace BitFlow::Core::Codegen
