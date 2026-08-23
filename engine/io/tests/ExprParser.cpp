#include "TestAssert.h"
#include "common/Assert.h"
#include "common/Expr.h"
#include "common/Rule.h"

#include <BitFlow/engine/io/ExprParser.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

inline bool EqualParseTree(const ExprStore* store, ExprId a, ExprId b) {

    const Expr& ea = (*store)[a];
    const Expr& eb = (*store)[b];

    if (ea.op != eb.op)
        return false;

    if (ea.inputs.size() != eb.inputs.size())
        return false;

    if (ea.bitWidth != eb.bitWidth)
        return false;

    if (ea.op == OpType::Const) {
        if (ea.knownMask != eb.knownMask)
            return false;

        if (ea.knownValue != eb.knownValue)
            return false;
    }

    for (std::size_t i = 0; i < ea.inputs.size(); ++i) {
        if (!EqualParseTree(store, ea.inputs[i], eb.inputs[i]))
            return false;
    }

    return true;
}

inline bool EqualParseTree(ExprRef a, ExprRef b) {
    BF_ASSERT(a.store == b.store);

    return EqualParseTree(a.store, a.id, b.id);
}

int TestExprParser_Precedence_MulBeforeAdd() {
    MakeExprStore(32);

    auto root = Parse("1 + 2 * 3").root;

    CPPTEST_ASSERT(Op(root) == OpType::Add);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Const);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Mul);
    return 0;
}

int TestExprParser_Precedence_AddBeforeShift() {
    MakeExprStore(32);

    auto root = Parse("1 << 2 + 3").root;

    CPPTEST_ASSERT(Op(root) == OpType::Shl);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Const);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Add);
    return 0;
}

int TestExprParser_Associativity_SubIsLeft() {
    MakeExprStore(32);

    auto root = Parse("1 - 2 - 3").root;

    CPPTEST_ASSERT(Op(root) == OpType::Sub);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Sub);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesUnaryAndBinaryMinus() {
    MakeExprStore(32);

    auto root = Parse("-a - b").root;

    CPPTEST_ASSERT(Op(root) == OpType::Sub);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Neg);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Var);
    return 0;
}

int TestExprParser_ParsesRotrCall() {
    MakeExprStore(32);

    auto root = Parse("a >>> 3").root;

    CPPTEST_ASSERT(Op(root) == OpType::RotR);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Var);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesRotlCall() {
    MakeExprStore(32);

    auto root = Parse("(x + y) <<< 5").root;

    CPPTEST_ASSERT(Op(root) == OpType::RotL);
    CPPTEST_ASSERT(InputSize(root) == 2);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Add);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesPowCall() {
    MakeExprStore(32);

    auto root = Parse("a ** 7").root;

    CPPTEST_ASSERT(Op(root) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(root) == 2);

    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Var);

    auto rhs = Input(root, 1);
    CPPTEST_ASSERT(Op(rhs) == OpType::Const);
    CPPTEST_ASSERT(EqualChunkValue(rhs, 7));
    return 0;
}

int TestExprParser_MixedExpressionShape() {
    MakeExprStore(32);

    auto root = Parse("~a ^ b & (c + 3) << 2").root;

    CPPTEST_ASSERT(Op(root) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(root) == 2);

    auto lhs = Input(root, 0);
    auto rhs = Input(root, 1);

    CPPTEST_ASSERT(Op(lhs) == OpType::Not);
    CPPTEST_ASSERT(Op(rhs) == OpType::And);

    auto shl = Input(rhs, 1);
    CPPTEST_ASSERT(Op(shl) == OpType::Shl);

    auto add = Input(shl, 0);
    CPPTEST_ASSERT(Op(add) == OpType::Add);

    return 0;
}

int TestExprParser_ShiftOperators() {
    MakeExprStore(32);

    auto root = Parse("a << b >> c").root;

    CPPTEST_ASSERT(Op(root) == OpType::Shr);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Shl);
    return 0;
}

int TestExprParser_ParsesLetterXAsMultiplicationInfix() {
    MakeExprStore(32);

    auto root = Parse("a x b").root;

    CPPTEST_ASSERT(Op(root) == OpType::Mul);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Var);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Var);
    return 0;
}

int TestExprParser_ParsesLetterXAsMultiplicationWithoutSpaces() {
    MakeExprStore(32);

    auto root = Parse("2x3").root;

    CPPTEST_ASSERT(Op(root) == OpType::Mul);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Const);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesVariableNamedX() {
    MakeExprStore(32);

    auto parsed = Parse("x + x1");

    CPPTEST_ASSERT(Op(parsed.root) == OpType::Add);
    CPPTEST_ASSERT(parsed.names.size() == 2);

    auto lhs = Input(parsed.root, 0);
    auto rhs = Input(parsed.root, 1);

    CPPTEST_ASSERT(parsed.names[lhs.id] == "x");
    CPPTEST_ASSERT(parsed.names[rhs.id] == "x1");
    return 0;
}

int TestExprParser_WidthConstructorsForVariables() {
    MakeExprStore(32);

    const struct {
        const char* text;
        BitFlow::Engine::Core::Types::BitWidth width;
    } cases[] = {{"u8(a)", 8}, {"u16(a)", 16}, {"u32(a)", 32}, {"u64(a)", 64}};

    for (const auto& c : cases) {
        auto parsed = Parse(c.text);

        CPPTEST_ASSERT(Op(parsed.root) == OpType::Var);
        CPPTEST_ASSERT(BitWidth(parsed.root) == c.width);
        CPPTEST_ASSERT(parsed.names.at(parsed.root.id) == "a");
    }

    {
        auto parsed = Parse("u8(a) + u8(b)");
        CPPTEST_ASSERT(Op(parsed.root) == OpType::Add);
        CPPTEST_ASSERT(BitWidth(parsed.root) == 8);
        CPPTEST_ASSERT(BitWidth(Input(parsed.root, 0)) == 8);
        CPPTEST_ASSERT(BitWidth(Input(parsed.root, 1)) == 8);
    }

    return 0;
}

int TestExprParser_WidthConstructorsForConstants() {
    MakeExprStore(32);

    {
        auto root = Parse("u8(255)").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(BitWidth(root) == 8);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 255);
    }

    {
        auto root = Parse("u16(255)").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(BitWidth(root) == 16);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 255);
    }

    return 0;
}

int TestExprParser_HexLiterals() {
    MakeExprStore(32);

    {
        auto root = Parse("0xFF").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 255);
    }

    {
        auto root = Parse("0x1234").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 0x1234);
    }

    return 0;
}

int TestExprParser_BinaryLiterals() {
    MakeExprStore(32);

    {
        auto root = Parse("0b10101010").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 0b10101010);
    }

    {
        auto root = Parse("0b11111111").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 0b11111111);
    }

    return 0;
}

int TestExprParser_WidthConstructorsForPrefixedLiterals() {
    MakeExprStore(32);

    {
        auto root = Parse("u8(0xFF)").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(BitWidth(root) == 8);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 255);
    }

    {
        auto root = Parse("u8(0b11111111)").root;
        CPPTEST_ASSERT(Op(root) == OpType::Const);
        CPPTEST_ASSERT(BitWidth(root) == 8);
        CPPTEST_ASSERT(ExprOf(root).knownValue == 255);
    }

    return 0;
}

int TestExprParser_WidthConstructorForCompoundExpression() {
    MakeExprStore(32);

    auto root = Parse("u32(a + b)").root;

    CPPTEST_ASSERT(Op(root) == OpType::Add);
    CPPTEST_ASSERT(BitWidth(root) == 32);
    CPPTEST_ASSERT(Op(Input(root, 0)) == OpType::Var);
    CPPTEST_ASSERT(Op(Input(root, 1)) == OpType::Var);

    return 0;
}

int TestExprParser_WidthConstructorNamesRemainVariablesWithoutCall() {
    MakeExprStore(32);

    auto parsed = Parse("u8 + 1");

    CPPTEST_ASSERT(Op(parsed.root) == OpType::Add);

    auto lhs = Input(parsed.root, 0);
    CPPTEST_ASSERT(Op(lhs) == OpType::Var);
    CPPTEST_ASSERT(parsed.names.at(lhs.id) == "u8");

    return 0;
}

int TestExprParser_RoundTrip_ToString() {
    MakeExprStore(32);

    auto original = Parse("1 << (2 + 3) ^ ~(a - b)").root;

    std::string text = ToString(original);

    auto reparsed = Parse(text).root;

    CPPTEST_ASSERT(EqualParseTree(original, reparsed));

    return 0;
}

int TestExprParser_UnaryChain_NormalizationAndRoundTrip() {
    MakeExprStore(32);

    {
        auto p = Parse("s---d+++4");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "s - d + 4");
    }

    {
        auto p = Parse("---a");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "-a");
    }

    {
        auto p = Parse("+++a");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "a");
    }

    {
        auto p = Parse("-+-a");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "a");
    }

    {
        auto p = Parse("a---b");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "a - b");
    }

    {
        auto p = Parse("a++++b");
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p.root, p.names) == "a + b");
    }

    {
        auto p1 = Parse("s---d+++4");
        auto s = BitFlow::Engine::IO::ToString(p1.root, p1.names);

        auto p2 = Parse(s);
        CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(p2.root, p2.names) == "s - d + 4");
    }

    return 0;
}

int main() {
    CPPTEST_RUN(TestExprParser_Precedence_MulBeforeAdd);
    CPPTEST_RUN(TestExprParser_Precedence_AddBeforeShift);
    CPPTEST_RUN(TestExprParser_Associativity_SubIsLeft);
    CPPTEST_RUN(TestExprParser_ParsesUnaryAndBinaryMinus);
    CPPTEST_RUN(TestExprParser_ParsesRotrCall);
    CPPTEST_RUN(TestExprParser_ParsesRotlCall);
    CPPTEST_RUN(TestExprParser_ParsesPowCall);
    CPPTEST_RUN(TestExprParser_MixedExpressionShape);
    CPPTEST_RUN(TestExprParser_ShiftOperators);
    CPPTEST_RUN(TestExprParser_ParsesLetterXAsMultiplicationInfix);
    CPPTEST_RUN(TestExprParser_ParsesLetterXAsMultiplicationWithoutSpaces);
    CPPTEST_RUN(TestExprParser_ParsesVariableNamedX);
    CPPTEST_RUN(TestExprParser_WidthConstructorsForVariables);
    CPPTEST_RUN(TestExprParser_WidthConstructorsForConstants);
    CPPTEST_RUN(TestExprParser_HexLiterals);
    CPPTEST_RUN(TestExprParser_BinaryLiterals);
    CPPTEST_RUN(TestExprParser_WidthConstructorsForPrefixedLiterals);
    CPPTEST_RUN(TestExprParser_WidthConstructorForCompoundExpression);
    CPPTEST_RUN(TestExprParser_WidthConstructorNamesRemainVariablesWithoutCall);
    CPPTEST_RUN(TestExprParser_RoundTrip_ToString);
    CPPTEST_RUN(TestExprParser_UnaryChain_NormalizationAndRoundTrip);
    return 0;
}
