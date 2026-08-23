#pragma once

#include <cstdint>

namespace BitFlow::Engine::Core::Types {

using BitWidth = uint16_t;

using ExprChunk = uint64_t;

constexpr BitWidth ExprChunkBits = static_cast<BitWidth>(sizeof(ExprChunk) * 8);

constexpr BitWidth ExprChunkBitsMinusOne = ExprChunkBits - 1;

} // namespace BitFlow::Core::Types
