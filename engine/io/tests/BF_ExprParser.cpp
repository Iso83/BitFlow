#include <BitFlow/io/ExprParser.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

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

    BF_TEST(Op(root) == OpType::Add);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Const);
    BF_TEST(Op(Input(root, 1)) == OpType::Mul);
    return 0;
}

int TestExprParser_Precedence_AddBeforeShift() {
    MakeExprStore(32);

    auto root = Parse("1 << 2 + 3").root;

    BF_TEST(Op(root) == OpType::Shl);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Const);
    BF_TEST(Op(Input(root, 1)) == OpType::Add);
    return 0;
}

int TestExprParser_Associativity_SubIsLeft() {
    MakeExprStore(32);

    auto root = Parse("1 - 2 - 3").root;

    BF_TEST(Op(root) == OpType::Sub);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Sub);
    BF_TEST(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesUnaryAndBinaryMinus() {
    MakeExprStore(32);

    auto root = Parse("-a - b").root;

    BF_TEST(Op(root) == OpType::Sub);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Neg);
    BF_TEST(Op(Input(root, 1)) == OpType::Var);
    return 0;
}

int TestExprParser_ParsesRotrCall() {
    MakeExprStore(32);

    auto root = Parse("a >>> 3").root;

    BF_TEST(Op(root) == OpType::RotR);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Var);
    BF_TEST(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesRotlCall() {
    MakeExprStore(32);

    auto root = Parse("(x + y) <<< 5").root;

    BF_TEST(Op(root) == OpType::RotL);
    BF_TEST(InputSize(root) == 2);
    BF_TEST(Op(Input(root, 0)) == OpType::Add);
    BF_TEST(Op(Input(root, 1)) == OpType::Const);
    return 0;
}

int TestExprParser_ParsesPowCall() {
    MakeExprStore(32);

    auto root = Parse("a ** 7").root;

    BF_TEST(Op(root) == OpType::Pow);
    BF_TEST(InputSize(root) == 2);

    BF_TEST(Op(Input(root, 0)) == OpType::Var);

    auto rhs = Input(root, 1);
    BF_TEST(Op(rhs) == OpType::Const);
    BF_TEST(EqualChunkValue(rhs, 7));
    return 0;
}

int TestExprParser_MixedExpressionShape() {
    MakeExprStore(32);

    auto root = Parse("~a ^ b & (c + 3) << 2").root;

    BF_TEST(Op(root) == OpType::Xor);
    BF_TEST(InputSize(root) == 2);

    auto lhs = Input(root, 0);
    auto rhs = Input(root, 1);

    BF_TEST(Op(lhs) == OpType::Not);
    BF_TEST(Op(rhs) == OpType::And);

    auto shl = Input(rhs, 1);
    BF_TEST(Op(shl) == OpType::Shl);

    auto add = Input(shl, 0);
    BF_TEST(Op(add) == OpType::Add);

    return 0;
}

int TestExprParser_ShiftOperators() {
    MakeExprStore(32);

    auto root = Parse("a << b >> c").root;

    BF_TEST(Op(root) == OpType::Shr);
    BF_TEST(Op(Input(root, 0)) == OpType::Shl);
    return 0;
}

int TestExprParser_RoundTrip_ToString() {
    MakeExprStore(32);

    auto original = Parse("1 << (2 + 3) ^ ~(a - b)").root;

    std::string text = ToString(original);

    auto reparsed = Parse(text).root;

    BF_TEST(EqualParseTree(original, reparsed));

    return 0;
}

int main() {
    BF_RUN_TEST(TestExprParser_Precedence_MulBeforeAdd);
    BF_RUN_TEST(TestExprParser_Precedence_AddBeforeShift);
    BF_RUN_TEST(TestExprParser_Associativity_SubIsLeft);
    BF_RUN_TEST(TestExprParser_ParsesUnaryAndBinaryMinus);
    BF_RUN_TEST(TestExprParser_ParsesRotrCall);
    BF_RUN_TEST(TestExprParser_ParsesRotlCall);
    BF_RUN_TEST(TestExprParser_ParsesPowCall);
    BF_RUN_TEST(TestExprParser_MixedExpressionShape);
    BF_RUN_TEST(TestExprParser_ShiftOperators);
    BF_RUN_TEST(TestExprParser_RoundTrip_ToString);
    return 0;
}
