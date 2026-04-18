#include "PerfPass.h"

#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <cstdint>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Codegen {

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
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];

        Key k{static_cast<uint32_t>(s.op), s.inputs};

        auto it = seen.find(k);
        if (it != seen.end()) {
            replace[s.id] = it->second;
        } else {
            seen[k] = s.id;
        }
    }

    for (auto& s : stmts)
        for (auto& in : s.inputs)
            if (replace.count(in))
                in = replace[in];
}


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
    for (auto& s : stmts)
        if (live.count(s.id))
            filtered.push_back(s);

    stmts.swap(filtered);
}

} // namespace BitFlow::Core::Codegen
