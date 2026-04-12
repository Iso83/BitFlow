#include "ExprKeyHash.h"

#include "ExprKey.h"

namespace BitFlow::Core::Expression {

std::size_t ExprKeyHash::operator()(const ExprKey& k) const {
    size_t h = std::hash<int>()((int)k.op);

    for (auto v : k.inputs)
        h ^= std::hash<uint32_t>()(v + 0x9e3779b9u + (h << 6) + (h >> 2));

    h ^= std::hash<bool>()(k.isConst);
    h ^= std::hash<uint32_t>()(k.constValue);

    return h;
}

} // namespace BitFlow::Core::Expression