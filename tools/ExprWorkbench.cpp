#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/eval/ConstantDetect.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

using namespace BitFlow;
using Expr = Core::AST::Expr;
using OpType = Core::AST::OpType;

struct CliOptions {
    std::string expr;
    uint32_t bitWidth = 32;

    bool normalize = false;
    bool simplify = false;
    bool factorize = false;

    bool ssa = false;
    bool emitC = false;
    bool emitFunc = false;
    bool eval = false;
    bool trace = false;
    bool verify = false;
    bool eq = false;
};

struct NamedExpr {
    std::string name;
    Expr* expr = nullptr;
};

const char* EvalStatusToString(Core::Eval::EvalStatus status) {
    using Core::Eval::EvalStatus;

    switch (status) {
    case EvalStatus::Success:
        return "Success";
    case EvalStatus::NotConstant:
        return "NotConstant";
    case EvalStatus::DivisionByZero:
        return "DivisionByZero";
    case EvalStatus::ModuloByZero:
        return "ModuloByZero";
    case EvalStatus::InvalidBitWidth:
        return "InvalidBitWidth";
    case EvalStatus::UnsupportedOp:
    default:
        return "UnsupportedOp";
    }
}

void PrintUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  bitflow_expr --expr \"...\" --bitwidth 32 [flags]\n\n";
    std::cerr << "Flags:\n";
    std::cerr << "  --normalize\n";
    std::cerr << "  --simplify\n";
    std::cerr << "  --factorize\n";
    std::cerr << "  --ssa\n";
    std::cerr << "  --emit-c\n";
    std::cerr << "  --emit-func\n";
    std::cerr << "  --eval\n";
    std::cerr << "  --trace\n";
    std::cerr << "  --verify\n";
    std::cerr << "  --eq\n";
}

bool ParseUInt32(const std::string& text, uint32_t& out) {
    try {
        const unsigned long value = std::stoul(text);
        if (value > 0xfffffffful)
            return false;
        out = static_cast<uint32_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

static std::string Trim(const std::string& in) {
    size_t a = 0;
    while (a < in.size() && std::isspace(static_cast<unsigned char>(in[a])))
        ++a;
    size_t b = in.size();
    while (b > a && std::isspace(static_cast<unsigned char>(in[b - 1])))
        --b;
    return in.substr(a, b - a);
}

CliOptions ParseArgs(int argc, char** argv) {
    CliOptions opt;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--expr") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --expr");
            opt.expr = argv[++i];
            continue;
        }

        if (arg == "--bitwidth") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --bitwidth");

            uint32_t bw = 0;
            if (!ParseUInt32(argv[++i], bw))
                throw std::runtime_error("Invalid --bitwidth value");

            opt.bitWidth = bw;
            continue;
        }

        if (arg == "--normalize") {
            opt.normalize = true;
            continue;
        }
        if (arg == "--simplify") {
            opt.simplify = true;
            continue;
        }
        if (arg == "--factorize") {
            opt.factorize = true;
            continue;
        }
        if (arg == "--ssa") {
            opt.ssa = true;
            continue;
        }
        if (arg == "--emit-c") {
            opt.emitC = true;
            continue;
        }
        if (arg == "--emit-func") {
            opt.emitFunc = true;
            continue;
        }
        if (arg == "--eval") {
            opt.eval = true;
            continue;
        }
        if (arg == "--trace") {
            opt.trace = true;
            continue;
        }
        if (arg == "--verify") {
            opt.verify = true;
            continue;
        }
        if (arg == "--eq") {
            opt.eq = true;
            continue;
        }

        throw std::runtime_error("Unknown argument: " + arg);
    }

    if (opt.expr.empty())
        throw std::runtime_error("Missing --expr");

    return opt;
}

void PrintSectionHeader(const char* title) {
    std::cout << "[" << title << "]\n";
}

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth == 0 || bitWidth > 64)
        return 0;
    if (bitWidth == 64)
        return ~uint64_t{0};
    return (uint64_t{1} << bitWidth) - 1ull;
}

uint64_t Mask(uint64_t value, uint32_t bitWidth) {
    return value & MaskFor(bitWidth);
}

uint32_t NormalizeShift(uint64_t amount, uint32_t bitWidth) {
    if (bitWidth == 0)
        return 0;
    return static_cast<uint32_t>(amount % static_cast<uint64_t>(bitWidth));
}

void CollectVars(const Expr* root, std::vector<uint32_t>& out) {
    if (!root)
        return;

    if (root->op == OpType::Var)
        out.push_back(root->id.value());

    for (const Expr* in : root->inputs)
        CollectVars(in, out);
}

Expr* MaterializeConstants(const Expr* root, const std::unordered_map<uint32_t, uint64_t>& env) {
    if (!root)
        return nullptr;

    if (root->op == OpType::Const)
        return Core::Expression::ConstPool::Get(root->constValue);

    if (root->op == OpType::Var) {
        auto it = env.find(root->id.value());
        const uint32_t value = (it == env.end()) ? 0u : static_cast<uint32_t>(it->second);
        return Core::Expression::ConstPool::Get(value);
    }

    Expr* n = new Expr{};
    n->op = root->op;
    for (const Expr* in : root->inputs)
        n->inputs.push_back(MaterializeConstants(in, env));
    return n;
}

Expr* CloneRebind(const Expr* root, const std::unordered_map<uint32_t, std::string>& localIdToName,
                 std::unordered_map<std::string, uint32_t>& globalNameToId,
                 std::unordered_map<uint32_t, std::string>& globalIdToName, uint32_t& nextGlobalId) {
    if (!root)
        return nullptr;

    if (root->op == OpType::Const)
        return Core::Expression::ConstPool::Get(root->constValue);

    if (root->op == OpType::Var) {
        const uint32_t localId = root->id.value();
        auto itName = localIdToName.find(localId);
        const std::string name = (itName == localIdToName.end()) ? ("v" + std::to_string(localId)) : itName->second;

        uint32_t globalId = 0;
        auto itGlobal = globalNameToId.find(name);
        if (itGlobal == globalNameToId.end()) {
            globalId = nextGlobalId++;
            globalNameToId[name] = globalId;
            globalIdToName[globalId] = name;
        } else {
            globalId = itGlobal->second;
        }

        Expr* v = new Expr{};
        v->op = OpType::Var;
        v->id = Core::Ids::ExprId{globalId};
        return v;
    }

    Expr* n = new Expr{};
    n->op = root->op;
    for (const Expr* in : root->inputs)
        n->inputs.push_back(CloneRebind(in, localIdToName, globalNameToId, globalIdToName, nextGlobalId));
    return n;
}

Expr* ExpandAliases(const Expr* root, const std::unordered_map<uint32_t, std::string>& idToName,
                   const std::unordered_map<std::string, Expr*>& aliases) {
    if (!root)
        return nullptr;

    if (root->op == OpType::Const)
        return Core::Expression::ConstPool::Get(root->constValue);

    if (root->op == OpType::Var) {
        auto itName = idToName.find(root->id.value());
        if (itName != idToName.end()) {
            auto itAlias = aliases.find(itName->second);
            if (itAlias != aliases.end())
                return itAlias->second;
        }

        Expr* v = new Expr{};
        v->op = OpType::Var;
        v->id = root->id;
        return v;
    }

    Expr* n = new Expr{};
    n->op = root->op;
    for (const Expr* in : root->inputs)
        n->inputs.push_back(ExpandAliases(in, idToName, aliases));
    return n;
}

std::vector<NamedExpr> ParseEquationMode(const std::string& text, std::unordered_map<uint32_t, std::string>& globalIdToName) {
    std::vector<NamedExpr> outputs;
    std::unordered_map<std::string, Expr*> aliases;
    std::unordered_map<std::string, uint32_t> globalNameToId;
    uint32_t nextGlobalId = 1;

    std::string normalized = text;
    std::replace(normalized.begin(), normalized.end(), ';', '\n');

    std::stringstream ss(normalized);
    std::string line;
    while (std::getline(ss, line)) {
        line = Trim(line);
        if (line.empty())
            continue;

        const size_t eqPos = line.find('=');
        if (eqPos == std::string::npos)
            throw std::runtime_error("Equation line missing '=': " + line);

        const std::string lhs = Trim(line.substr(0, eqPos));
        const std::string rhs = Trim(line.substr(eqPos + 1));
        if (lhs.empty() || rhs.empty())
            throw std::runtime_error("Invalid equation line: " + line);

        auto parsed = IO::Parse(rhs);
        Expr* rebound = CloneRebind(parsed.root, parsed.idToName, globalNameToId, globalIdToName, nextGlobalId);
        Expr* expanded = ExpandAliases(rebound, globalIdToName, aliases);

        aliases[lhs] = expanded;
        outputs.push_back({lhs, expanded});
    }

    if (outputs.empty())
        throw std::runtime_error("No equations parsed");

    return outputs;
}

std::string ReplaceVarTokens(std::string text, const std::unordered_map<uint32_t, std::string>& idToName) {
    auto isWord = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };

    for (const auto& [id, name] : idToName) {
        const std::string from = "v" + std::to_string(id);
        size_t pos = 0;
        while ((pos = text.find(from, pos)) != std::string::npos) {
            const bool leftOk = (pos == 0) || !isWord(text[pos - 1]);
            const size_t rp = pos + from.size();
            const bool rightOk = (rp >= text.size()) || !isWord(text[rp]);
            if (leftOk && rightOk) {
                text.replace(pos, from.size(), name);
                pos += name.size();
            } else {
                pos += from.size();
            }
        }
    }

    return text;
}

Core::Eval::EvalResult EvalWithEnv(const Expr* root, const std::unordered_map<uint32_t, uint64_t>& env, uint32_t bitWidth) {
    using namespace Core::Eval;

    if (!root)
        return {EvalStatus::UnsupportedOp, 0};

    if (bitWidth == 0 || bitWidth > 64)
        return {EvalStatus::InvalidBitWidth, 0};

    if (root->op == OpType::Const)
        return {EvalStatus::Success, Mask(root->constValue, bitWidth)};

    if (root->op == OpType::Var) {
        auto it = env.find(root->id.value());
        const uint64_t value = (it == env.end()) ? 0ull : it->second;
        return {EvalStatus::Success, Mask(value, bitWidth)};
    }

    std::vector<uint64_t> in;
    in.reserve(root->inputs.size());
    for (const Expr* child : root->inputs) {
        EvalResult r = EvalWithEnv(child, env, bitWidth);
        if (r.status != EvalStatus::Success)
            return r;
        in.push_back(Mask(r.value, bitWidth));
    }

    const uint64_t mask = MaskFor(bitWidth);
    switch (root->op) {
    case OpType::Not:
        if (in.size() != 1)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, (~in[0]) & mask};
    case OpType::Neg:
        if (in.size() != 1)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, ((~in[0]) + 1ull) & mask};
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
        if (in.empty())
            return {EvalStatus::UnsupportedOp, 0};
        uint64_t acc = in[0] & mask;
        for (size_t i = 1; i < in.size(); ++i) {
            switch (root->op) {
            case OpType::And:
                acc &= in[i];
                break;
            case OpType::Or:
                acc |= in[i];
                break;
            case OpType::Xor:
                acc ^= in[i];
                break;
            case OpType::Add:
                acc += in[i];
                break;
            case OpType::Mul:
                acc *= in[i];
                break;
            default:
                break;
            }
            acc &= mask;
        }
        return {EvalStatus::Success, acc};
    }
    case OpType::Sub:
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, (in[0] - in[1]) & mask};
    case OpType::Div:
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        if (in[1] == 0)
            return {EvalStatus::DivisionByZero, 0};
        return {EvalStatus::Success, (in[0] / in[1]) & mask};
    case OpType::Mod:
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        if (in[1] == 0)
            return {EvalStatus::ModuloByZero, 0};
        return {EvalStatus::Success, (in[0] % in[1]) & mask};
    case OpType::Shl:
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, (in[0] << NormalizeShift(in[1], bitWidth)) & mask};
    case OpType::Shr:
    case OpType::UShr:
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, (in[0] >> NormalizeShift(in[1], bitWidth)) & mask};
    case OpType::RotL: {
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        const uint32_t s = NormalizeShift(in[1], bitWidth);
        if (s == 0)
            return {EvalStatus::Success, in[0] & mask};
        return {EvalStatus::Success, ((in[0] << s) | (in[0] >> (bitWidth - s))) & mask};
    }
    case OpType::RotR: {
        if (in.size() != 2)
            return {EvalStatus::UnsupportedOp, 0};
        const uint32_t s = NormalizeShift(in[1], bitWidth);
        if (s == 0)
            return {EvalStatus::Success, in[0] & mask};
        return {EvalStatus::Success, ((in[0] >> s) | (in[0] << (bitWidth - s))) & mask};
    }
    case OpType::Ch:
        if (in.size() != 3)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, ((in[0] & in[1]) ^ ((~in[0]) & in[2])) & mask};
    case OpType::Maj:
        if (in.size() != 3)
            return {EvalStatus::UnsupportedOp, 0};
        return {EvalStatus::Success, ((in[0] & in[1]) ^ (in[0] & in[2]) ^ (in[1] & in[2])) & mask};
    default:
        return {EvalStatus::UnsupportedOp, 0};
    }
}

void RunVerify(const Expr* rewritten, uint32_t bitWidth) {
    std::vector<uint32_t> vars;
    CollectVars(rewritten, vars);
    std::sort(vars.begin(), vars.end());
    vars.erase(std::unique(vars.begin(), vars.end()), vars.end());

    std::mt19937_64 rng{0xB17F10ULL};
    std::uniform_int_distribution<uint64_t> dist(0, MaskFor(bitWidth));

    constexpr int kCases = 128;
    int passed = 0;

    for (int i = 0; i < kCases; ++i) {
        std::unordered_map<uint32_t, uint64_t> env;
        for (uint32_t id : vars)
            env[id] = dist(rng);

        Expr* constTree = MaterializeConstants(rewritten, env);
        const auto evalResult = Core::Eval::EvaluateConstant(constTree, bitWidth);
        const auto simResult = EvalWithEnv(rewritten, env, bitWidth);

        const bool okStatus = (evalResult.status == simResult.status);
        const bool okValue = (evalResult.status != Core::Eval::EvalStatus::Success) ||
                             (Mask(evalResult.value, bitWidth) == Mask(simResult.value, bitWidth));

        if (okStatus && okValue) {
            ++passed;
            continue;
        }

        std::cout << "case " << i << ": mismatch\n";
    }

    std::cout << "cases=" << kCases << ", passed=" << passed << ", failed=" << (kCases - passed) << "\n\n";
}

Core::Rules::RuleEngine BuildRuleEngine(const CliOptions& opt, const std::unordered_map<uint32_t, std::string>& names) {
    const bool hasStageSelection = opt.normalize || opt.simplify || opt.factorize;
    const bool runNormalize = hasStageSelection ? opt.normalize : true;
    const bool runSimplify = hasStageSelection ? opt.simplify : true;
    const bool runFactorize = hasStageSelection ? opt.factorize : false;

    Core::Rules::RuleEngine engine;
    if (runNormalize)
        Core::Rules::Add_Normalize_Rules(engine);
    if (runSimplify) {
        Core::Rules::Add_Simplify_Bitwise_Rules(engine);
        Core::Rules::Add_Simplify_Arithmetic_Rules(engine);
    }
    if (runFactorize) {
        Core::Rules::Add_Factorize_Bitwise_Rules(engine);
        Core::Rules::Add_Factorize_Arithmetic_Rules(engine);
    }

    if (opt.trace) {
        engine.SetDebugCallback([&](const Expr* before, const Expr* after, Core::Rules::RuleId id) {
            std::cout << "[trace] [" << static_cast<int>(id) << "] " << IO::ToString(before, names) << " -> "
                      << IO::ToString(after, names) << "\n";
        });
    }

    return engine;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const CliOptions opt = ParseArgs(argc, argv);

        std::unordered_map<uint32_t, std::string> names;
        std::vector<NamedExpr> outputs;

        if (opt.eq) {
            outputs = ParseEquationMode(opt.expr, names);

            PrintSectionHeader("parsed");
            for (const auto& out : outputs)
                std::cout << out.name << " = " << IO::ToString(out.expr, names) << "\n";
            std::cout << "\n";
        } else {
            auto parsed = IO::Parse(opt.expr);
            names = parsed.idToName;
            outputs.push_back({"out", parsed.root});

            PrintSectionHeader("parsed");
            std::cout << IO::ToString(parsed.root, names) << "\n\n";
        }

        auto engine = BuildRuleEngine(opt, names);

        for (auto& out : outputs)
            out.expr = engine.ApplyUntilStable(out.expr);

        PrintSectionHeader("rewritten");
        for (const auto& out : outputs) {
            if (opt.eq)
                std::cout << out.name << " = ";
            std::cout << IO::ToString(out.expr, names) << "\n";
        }
        std::cout << "\n";

        if (opt.ssa) {
            PrintSectionHeader("ssa");
            for (const auto& out : outputs) {
                if (opt.eq)
                    std::cout << "# " << out.name << "\n";
                Core::Codegen::SsaProgram ssa = Core::Codegen::BuildSSA(out.expr, opt.bitWidth);
                for (const auto& st : ssa.statements)
                    std::cout << st.name << " = " << ReplaceVarTokens(st.expr, names) << "\n";
                std::cout << "result = " << ReplaceVarTokens(ssa.result.empty() ? std::string("0") : ssa.result, names)
                          << "\n";
            }
            std::cout << "\n";
        }

        if (opt.emitC) {
            PrintSectionHeader("c-expr");
            for (const auto& out : outputs) {
                const std::string exprC = ReplaceVarTokens(Core::Codegen::EmitCExpr(out.expr, opt.bitWidth), names);
                if (opt.eq)
                    std::cout << out.name << " = ";
                std::cout << exprC << "\n";
            }
            std::cout << "\n";
        }

        if (opt.emitFunc) {
            PrintSectionHeader("c-func");
            if (!opt.eq || outputs.size() == 1) {
                std::cout << ReplaceVarTokens(Core::Codegen::EmitCFunction(outputs[0].expr, opt.bitWidth), names) << "\n\n";
            } else {
                std::vector<uint32_t> varVec;
                for (const auto& out : outputs)
                    CollectVars(out.expr, varVec);
                std::sort(varVec.begin(), varVec.end());
                varVec.erase(std::unique(varVec.begin(), varVec.end()), varVec.end());

                const std::string ctype = (opt.bitWidth <= 32U) ? "uint32_t" : ((opt.bitWidth <= 64U) ? "uint64_t" : "bf_uint");

                std::cout << "struct Outputs {\n";
                for (const auto& out : outputs)
                    std::cout << "    " << ctype << " " << out.name << ";\n";
                std::cout << "};\n\n";
                std::cout << "Outputs f(";
                for (size_t i = 0; i < varVec.size(); ++i) {
                    if (i)
                        std::cout << ", ";
                    const uint32_t id = varVec[i];
                    std::cout << ctype << " " << names[id];
                }
                std::cout << ") {\n";
                std::cout << "    Outputs r{};\n";
                for (const auto& out : outputs)
                    std::cout << "    r." << out.name << " = "
                              << ReplaceVarTokens(Core::Codegen::EmitCExpr(out.expr, opt.bitWidth), names) << ";\n";
                std::cout << "    return r;\n}" << "\n\n";
            }
        }

        if (opt.eval && !opt.eq) {
            PrintSectionHeader("eval");
            if (!Core::Eval::IsFullyConstant(outputs[0].expr)) {
                std::cout << "NotConstant\n\n";
            } else {
                const auto result = Core::Eval::EvaluateConstant(outputs[0].expr, opt.bitWidth);
                if (result.status == Core::Eval::EvalStatus::Success)
                    std::cout << result.value << "\n\n";
                else
                    std::cout << EvalStatusToString(result.status) << "\n\n";
            }
        }

        if (opt.verify && !opt.eq) {
            PrintSectionHeader("verify");
            RunVerify(outputs[0].expr, opt.bitWidth);
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        PrintUsage();
        return 1;
    }
}
