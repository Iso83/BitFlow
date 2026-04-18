#include "PerfPass.h"

#include <BitFlow/core/ast/OpType.h>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace BitFlow::Core::Codegen {
namespace {
constexpr uint32_t kVarTag = 0x40000000u;
constexpr uint32_t kConstTag = 0x80000000u;

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth == 64U)
        return ~uint64_t{0};
    return (uint64_t{1} << bitWidth) - 1ULL;
}

uint64_t Mask(uint64_t v, uint32_t bw) {
    if (bw >= 64U)
        return v;
    return v & ((1ULL << bw) - 1ULL);
}

uint32_t NormalizeShift(uint64_t amount, uint32_t bitWidth) {
    return static_cast<uint32_t>(amount % static_cast<uint64_t>(bitWidth));
}

bool TryFoldPureOp(uint32_t op, const std::vector<uint64_t>& in, uint32_t bitWidth, uint64_t& outValue) {
    const uint64_t mask = MaskFor(bitWidth);
    const AST::OpType opType = static_cast<AST::OpType>(op);

    switch (opType) {
    case AST::OpType::Not:
        if (in.size() != 1)
            return false;
        outValue = (~in[0]) & mask;
        return true;
    case AST::OpType::Neg:
        if (in.size() != 1)
            return false;
        outValue = (~in[0] + 1ULL) & mask;
        return true;
    case AST::OpType::And:
    case AST::OpType::Or:
    case AST::OpType::Xor:
    case AST::OpType::Add:
    case AST::OpType::Mul:
        if (in.empty())
            return false;
        outValue = in[0] & mask;
        for (size_t i = 1; i < in.size(); ++i) {
            switch (opType) {
            case AST::OpType::And:
                outValue &= in[i];
                break;
            case AST::OpType::Or:
                outValue |= in[i];
                break;
            case AST::OpType::Xor:
                outValue ^= in[i];
                break;
            case AST::OpType::Add:
                outValue += in[i];
                break;
            case AST::OpType::Mul:
                outValue *= in[i];
                break;
            default:
                return false;
            }
            outValue &= mask;
        }
        return true;
    case AST::OpType::Sub:
    case AST::OpType::Div:
    case AST::OpType::Mod:
    case AST::OpType::Shl:
    case AST::OpType::Shr:
    case AST::OpType::UShr:
    case AST::OpType::RotL:
    case AST::OpType::RotR:
        if (in.size() != 2)
            return false;
        switch (opType) {
        case AST::OpType::Sub:
            outValue = (in[0] - in[1]) & mask;
            return true;
        case AST::OpType::Div:
            if (in[1] == 0)
                return false;
            outValue = (in[0] / in[1]) & mask;
            return true;
        case AST::OpType::Mod:
            if (in[1] == 0)
                return false;
            outValue = (in[0] % in[1]) & mask;
            return true;
        case AST::OpType::Shl:
            outValue = (in[0] << NormalizeShift(in[1], bitWidth)) & mask;
            return true;
        case AST::OpType::Shr:
        case AST::OpType::UShr:
            outValue = (in[0] >> NormalizeShift(in[1], bitWidth)) & mask;
            return true;
        case AST::OpType::RotL: {
            const uint32_t shift = NormalizeShift(in[1], bitWidth);
            if (shift == 0) {
                outValue = in[0] & mask;
                return true;
            }
            outValue = ((in[0] << shift) | (in[0] >> (bitWidth - shift))) & mask;
            return true;
        }
        case AST::OpType::RotR: {
            const uint32_t shift = NormalizeShift(in[1], bitWidth);
            if (shift == 0) {
                outValue = in[0] & mask;
                return true;
            }
            outValue = ((in[0] >> shift) | (in[0] << (bitWidth - shift))) & mask;
            return true;
        }
        default:
            return false;
        }
    case AST::OpType::Ch:
    case AST::OpType::Maj:
        if (in.size() != 3)
            return false;
        if (opType == AST::OpType::Ch) {
            outValue = ((in[0] & in[1]) ^ ((~in[0]) & in[2])) & mask;
            return true;
        }
        outValue = ((in[0] & in[1]) ^ (in[0] & in[2]) ^ (in[1] & in[2])) & mask;
        return true;
    case AST::OpType::Var:
    case AST::OpType::Const:
    default:
        return false;
    }
}

bool EvalOp(uint32_t op, const std::vector<uint64_t>& in, uint64_t& out, uint32_t bw) {
    if (bw == 0 || bw > 64)
        return false;

    if (in.size() < 2)
        return false;

    const uint64_t a = Mask(in[0], bw);
    const uint32_t shift = static_cast<uint32_t>(in[1] % static_cast<uint64_t>(bw));

    switch (op) {
    case (uint32_t)AST::OpType::Add:
        out = Mask(a + in[1], bw);
        return true;
    case (uint32_t)AST::OpType::Sub:
        out = Mask(a - in[1], bw);
        return true;
    case (uint32_t)AST::OpType::Mul:
        out = Mask(a * in[1], bw);
        return true;
    case (uint32_t)AST::OpType::And:
        out = Mask(a & in[1], bw);
        return true;
    case (uint32_t)AST::OpType::Or:
        out = Mask(a | in[1], bw);
        return true;
    case (uint32_t)AST::OpType::Xor:
        out = Mask(a ^ in[1], bw);
        return true;
    case (uint32_t)AST::OpType::Shl:
        out = Mask(a << shift, bw);
        return true;
    case (uint32_t)AST::OpType::Shr:
    case (uint32_t)AST::OpType::UShr:
        out = Mask(a >> shift, bw);
        return true;
    case (uint32_t)AST::OpType::RotL: {
        if (shift == 0) {
            out = a;
            return true;
        }
        out = Mask((a << shift) | (a >> (bw - shift)), bw);
        return true;
    }
    case (uint32_t)AST::OpType::RotR: {
        if (shift == 0) {
            out = a;
            return true;
        }
        out = Mask((a >> shift) | (a << (bw - shift)), bw);
        return true;
    }
    default:
        return false;
    }
}

uint32_t FindOrCreateConstId(std::unordered_map<uint32_t, uint64_t>& constValues, uint64_t value) {
    for (const auto& [id, v] : constValues)
        if (v == value)
            return id;

    uint32_t nextConstIndex = 0;
    for (const auto& [id, _] : constValues) {
        if ((id & kConstTag) == 0u)
            continue;
        uint32_t index = id & ~kConstTag;
        if (index >= nextConstIndex)
            nextConstIndex = index + 1;
    }

    const uint32_t newId = kConstTag | nextConstIndex;
    constValues[newId] = value;
    return newId;
}

void ApplyConstantFoldWithMap(std::vector<Statement>& stmts, uint32_t& rootId,
                              std::unordered_map<uint32_t, uint64_t>& constValues, uint32_t bitWidth) {
    if (bitWidth == 0 || bitWidth > 64)
        return;

    const uint64_t mask = MaskFor(bitWidth);
    std::unordered_map<uint32_t, uint32_t> replace;
    std::vector<Statement> folded;
    folded.reserve(stmts.size());

    for (auto& s : stmts) {
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];

        bool allInputsConst = !s.inputs.empty();
        std::vector<uint64_t> inValues;
        inValues.reserve(s.inputs.size());
        for (uint32_t in : s.inputs) {
            auto it = constValues.find(in);
            if (it == constValues.end()) {
                allInputsConst = false;
                break;
            }
            inValues.push_back(it->second & mask);
        }

        if (!allInputsConst) {
            folded.push_back(s);
            continue;
        }

        uint64_t out = 0;
        if (!TryFoldPureOp(s.op, inValues, bitWidth, out)) {
            folded.push_back(s);
            continue;
        }

        const uint32_t constId = FindOrCreateConstId(constValues, out & mask);
        replace[s.id] = constId;
    }

    for (auto& s : folded)
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];

    if (replace.count(rootId))
        rootId = replace[rootId];

    stmts.swap(folded);
}
}

// ============================
// CSE
// ============================

struct Key {
    uint32_t op;
    std::vector<uint32_t> inputs;

    bool operator==(const Key& o) const {
        return op == o.op && inputs == o.inputs;
    }
};

struct KeyHash {
    size_t operator()(const Key& k) const {
        size_t h = std::hash<uint32_t>{}(k.op);
        for (auto v : k.inputs)
            h ^= std::hash<uint32_t>{}(v + 0x9e3779b9 + (h << 6) + (h >> 2));
        return h;
    }
};

void ApplyCSE(std::vector<Statement>& stmts) {
    std::unordered_map<Key, uint32_t, KeyHash> seen;
    std::unordered_map<uint32_t, uint32_t> replace;

    for (auto& s : stmts) {
        // remap inputs first
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];

        Key k{s.op, s.inputs};

        auto it = seen.find(k);
        if (it != seen.end()) {
            replace[s.id] = it->second;
        } else {
            seen[k] = s.id;
        }
    }

    // second pass
    for (auto& s : stmts)
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];
}

void ApplyConstantFolding(std::vector<Statement>& stmts, uint32_t bitWidth) {
    if (bitWidth == 0 || bitWidth > 64)
        return;

    std::unordered_map<uint32_t, uint64_t> constValues;
    std::unordered_map<uint32_t, uint32_t> replace;

    for (const auto& s : stmts)
        for (uint32_t in : s.inputs)
            if ((in & kConstTag) != 0u)
                constValues[in] = static_cast<uint64_t>(in & ~kConstTag);

    for (const auto& s : stmts)
        if (constValues.count(s.id))
            constValues[s.id] = Mask(constValues[s.id], bitWidth);

    for (auto& s : stmts) {
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];

        std::vector<uint64_t> inVals;
        bool allConst = true;
        for (auto in : s.inputs) {
            if (!constValues.count(in)) {
                allConst = false;
                break;
            }
            inVals.push_back(constValues[in]);
        }

        if (!allConst)
            continue;

        uint64_t out = 0;
        if (!EvalOp(s.op, inVals, out, bitWidth))
            continue;

        constValues[s.id] = out;
        replace[s.id] = s.id;
        s.inputs.clear();
    }
}

// ============================
// DCE
// ============================

void ApplyDCE(std::vector<Statement>& stmts, uint32_t rootId) {
    std::unordered_set<uint32_t> live;

    std::function<void(uint32_t)> mark = [&](uint32_t id) {
        if (!live.insert(id).second)
            return;

        for (auto& s : stmts)
            if (s.id == id)
                for (auto in : s.inputs)
                    mark(in);
    };

    mark(rootId);

    std::vector<Statement> filtered;
    filtered.reserve(stmts.size());

    for (auto& s : stmts)
        if (live.count(s.id))
            filtered.push_back(s);

    stmts.swap(filtered);
}

// ============================
// Temp reuse
// ============================

void ApplyTempReuse(std::vector<Statement>& stmts) {
    std::unordered_map<uint32_t, int> useCount;

    auto isTempId = [](uint32_t id) { return (id & (kVarTag | kConstTag)) == 0u; };

    for (auto& s : stmts)
        for (auto in : s.inputs)
            if (isTempId(in))
                useCount[in]++;

    std::queue<uint32_t> freeTemps;
    std::unordered_map<uint32_t, uint32_t> remap;

    for (auto& s : stmts) {
        // remap inputs
        for (auto& in : s.inputs) {
            if (remap.count(in))
                in = remap[in];

            if (!isTempId(in))
                continue;

            if (--useCount[in] == 0)
                freeTemps.push(in);
        }

        // reuse slot
        if (!freeTemps.empty()) {
            uint32_t reuse = freeTemps.front();
            freeTemps.pop();

            remap[s.id] = reuse;
            s.id = reuse;
        }
    }
}

// ============================
// Pipeline
// ============================

void ApplyPerfPass(std::vector<Statement>& stmts, uint32_t rootId) {
    ApplyConstantFolding(stmts, 64U);
    ApplyCSE(stmts);
    ApplyDCE(stmts, rootId);
    ApplyTempReuse(stmts);
}

void ApplyPerfPass(std::vector<Statement>& stmts, uint32_t& rootId, std::unordered_map<uint32_t, uint64_t>& constValues,
                   uint32_t bitWidth) {
    ApplyConstantFoldWithMap(stmts, rootId, constValues, bitWidth);
    ApplyCSE(stmts);
    ApplyDCE(stmts, rootId);
    ApplyTempReuse(stmts);
}

} // namespace BitFlow::Core::Codegen
