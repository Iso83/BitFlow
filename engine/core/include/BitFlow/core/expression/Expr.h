#pragma once

#ifdef BF_EXPR_LIFETIME_CHECKS
#pragma detect_mismatch("BF_EXPR_LIFETIME_CHECKS", "ON")
#else
#pragma detect_mismatch("BF_EXPR_LIFETIME_CHECKS", "OFF")
#endif

#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/helper/CheckedExprInputs.h>
#include <BitFlow/core/helper/FieldHook.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/types/Types.h>
#include <vector>

namespace BitFlow::Core::Expression {

class ExprStore;

#ifdef BF_EXPR_LIFETIME_CHECKS
struct ExprUnsafeStorage {
#else
struct Expr {
#endif
    OpType op{};
    std::vector<Ids::ExprId> inputs{};

    Types::BitWidth bitWidth{0};

    Types::ExprChunk knownMask{0};
    Types::ExprChunk knownValue{0};

    uint32_t largeConstIndex{0};

  public:
    [[nodiscard]] static Types::ExprChunk fullMask(Types::BitWidth bitWidth) {
        if (bitWidth == 0)
            return 0;

        if (bitWidth >= Types::ExprChunkBits)
            return ~Types::ExprChunk{0};

        return (Types::ExprChunk{1} << bitWidth) - 1;
    }
};

#ifdef BF_EXPR_LIFETIME_CHECKS

struct ExprDebug {
  private:
    template <typename T> friend class FieldHook;
    friend class ExprStore;
    friend class CheckedExprInputs;

    ExprStore* m_store{};
    Ids::ExprId m_id{};
    ExprUnsafeStorage* m_expr{};
    Types::ExprChunk m_generation{};

    void SanityCheck() const;

  public:
    FieldHook<OpType> op{this, &ExprUnsafeStorage::op};
    CheckedExprInputs inputs{this};

    FieldHook<Types::BitWidth> bitWidth{this, &ExprUnsafeStorage::bitWidth};

    FieldHook<Types::ExprChunk> knownMask{this, &ExprUnsafeStorage::knownMask};
    FieldHook<Types::ExprChunk> knownValue{this, &ExprUnsafeStorage::knownValue};

    FieldHook<uint32_t> largeConstIndex{this, &ExprUnsafeStorage::largeConstIndex};

  public:
    [[nodiscard]] static Types::ExprChunk fullMask(Types::BitWidth bitWidth) {
        return ExprUnsafeStorage::fullMask(bitWidth);
    }
};

using Expr = ExprDebug;
#endif

} // namespace BitFlow::Core::Expression
