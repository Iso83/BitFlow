#include <BitFlow/core/expression/ExprRef.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <algorithm>

namespace BitFlow::Core::Expression {

ExprRef::ExprRef(ExprStore* owner, Ids::ExprId exprId) : store(owner), id(exprId) {}

bool ExprRef::IsValid() const noexcept {
    return store != nullptr && id.value() != 0;
}

bool ExprRef::operator==(const ExprRef& other) const noexcept {
    return store == other.store && id == other.id;
}

bool ExprRef::operator!=(const ExprRef& other) const noexcept {
    return !(*this == other);
}

Types::BitWidth ExprRef::BitWidth() const {
    return store->get(*this).bitWidth;
}

ExprRef ExprRef::Const(Types::ExprChunk value, Types::BitWidth bitWidth) const {
    if (bitWidth == 0)
        bitWidth = Types::ExprChunkBits;

    return store->createConstant(value, bitWidth);
}

ExprRef ExprRef::operator~() const {
    return store->create(OpType::Not, {id}, BitWidth());
}

ExprRef ExprRef::operator-() const {
    return store->create(OpType::Neg, {id}, BitWidth());
}

ExprRef ExprRef::operator+(ExprRef rhs) const {
    return store->create(OpType::Add, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator-(ExprRef rhs) const {
    return store->create(OpType::Sub, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator*(ExprRef rhs) const {
    return store->create(OpType::Mul, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator/(ExprRef rhs) const {
    return store->create(OpType::Div, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator%(ExprRef rhs) const {
    return store->create(OpType::Mod, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator<<(ExprRef rhs) const {
    return store->create(OpType::Shl, {id, rhs.id}, BitWidth());
}

ExprRef ExprRef::operator>>(ExprRef rhs) const {
    return store->create(OpType::Shr, {id, rhs.id}, BitWidth());
}

ExprRef ExprRef::RotL(ExprRef rhs) const {
    return store->create(OpType::RotL, {id, rhs.id}, BitWidth());
}

ExprRef ExprRef::RotR(ExprRef rhs) const {
    return store->create(OpType::RotR, {id, rhs.id}, BitWidth());
}

ExprRef ExprRef::operator&(ExprRef rhs) const {
    return store->create(OpType::And, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator^(ExprRef rhs) const {
    return store->create(OpType::Xor, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator|(ExprRef rhs) const {
    return store->create(OpType::Or, {id, rhs.id}, std::max(BitWidth(), rhs.BitWidth()));
}

ExprRef ExprRef::operator+(Types::ExprChunk rhs) const {
    return *this + Const(rhs, BitWidth());
}

ExprRef ExprRef::operator-(Types::ExprChunk rhs) const {
    return *this - Const(rhs, BitWidth());
}

ExprRef ExprRef::operator*(Types::ExprChunk rhs) const {
    return *this * Const(rhs, BitWidth());
}

ExprRef ExprRef::operator/(Types::ExprChunk rhs) const {
    return *this / Const(rhs, BitWidth());
}

ExprRef ExprRef::operator%(Types::ExprChunk rhs) const {
    return *this % Const(rhs, BitWidth());
}

ExprRef ExprRef::operator<<(Types::ExprChunk rhs) const {
    return *this << Const(rhs, BitWidth());
}

ExprRef ExprRef::operator>>(Types::ExprChunk rhs) const {
    return *this >> Const(rhs, BitWidth());
}

ExprRef ExprRef::RotL(Types::ExprChunk rhs) const {
    return RotL(Const(rhs, BitWidth()));
}

ExprRef ExprRef::RotR(Types::ExprChunk rhs) const {
    return RotR(Const(rhs, BitWidth()));
}

ExprRef ExprRef::operator&(Types::ExprChunk rhs) const {
    return *this & Const(rhs, BitWidth());
}

ExprRef ExprRef::operator^(Types::ExprChunk rhs) const {
    return *this ^ Const(rhs, BitWidth());
}

ExprRef ExprRef::operator|(Types::ExprChunk rhs) const {
    return *this | Const(rhs, BitWidth());
}

} // namespace BitFlow::Core::Expression