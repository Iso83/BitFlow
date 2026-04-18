#include "PerfPass.h"

#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace BitFlow::Core::Codegen {
namespace {
constexpr uint32_t kVarTag = 0x40000000u;
constexpr uint32_t kConstTag = 0x80000000u;
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
    ApplyCSE(stmts);
    ApplyDCE(stmts, rootId);
    ApplyTempReuse(stmts);
}

} // namespace BitFlow::Core::Codegen
