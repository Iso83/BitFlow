#pragma once

#include <cstdint>
#include <string>

namespace BitFlow::Core::Codegen {

inline std::string GetCType(uint32_t bw) {
    if (bw <= 32U)
        return "uint32_t";
    if (bw <= 64U)
        return "uint64_t";
    return "bf_uint"; // placeholder (Stap 17)
}

} // namespace BitFlow::Core::Codegen
