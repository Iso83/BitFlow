#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Arithmetic_Rules(engine);
    Add_Factorize_Arithmetic_Rules(engine);
    return engine;
}

static bool HasCoeffBaseMul(Expr* expr, Expr* base, uint32_t coeff) {
    if (expr->op != OpType::Mul)
        return false;

    bool hasBase = false;
    bool hasCoeff = false;
    for (Expr* in : expr->inputs) {
        if (in->id == base->id)
            hasBase = true;
        if (in->op == OpType::Const && in->constValue == coeff)
            hasCoeff = true;
    }

    return hasBase && hasCoeff;
}

int TestLinearMultiplicity_BasicAndMixed() {
    RuleEngine engine = MakeArithmeticEngine();
    auto a = MakeVar(1000);

    {
        auto expr = MakeOp(1001, OpType::Add, {a, a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 2u));
    }

    {
        auto expr = MakeOp(1002, OpType::Add, {a, a, a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 3u));
    }

    {
        auto expr = MakeOp(1003, OpType::Add, {a, MakeOp(1004, OpType::Mul, {a, MakeConst(1005, 2)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 3u));
    }

    {
        auto expr = MakeOp(1006, OpType::Add, {MakeOp(1007, OpType::Mul, {a, MakeConst(1008, 2)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 3u));
    }

    {
        auto expr = MakeOp(
            1009, OpType::Add,
            {MakeOp(1010, OpType::Mul, {a, MakeConst(1011, 2)}), MakeOp(1012, OpType::Mul, {a, MakeConst(1013, 3)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 5u));
    }

    {
        auto expr = MakeOp(1014, OpType::Add, {MakeOp(1015, OpType::Mul, {a, MakeConst(1016, 0)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->id == a->id);
    }

    {
        auto expr = MakeOp(
            1017, OpType::Add,
            {MakeOp(1018, OpType::Mul, {a, MakeConst(1019, 1)}), MakeOp(1020, OpType::Mul, {a, MakeConst(1021, 2)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(HasCoeffBaseMul(result, a, 3u));
    }

    return 0;
}

int TestLinearMultiplicity_Guards() {
    RuleEngine engine = MakeArithmeticEngine();
    auto a = MakeVar(1030);
    auto b = MakeVar(1031);
    auto c = MakeVar(1032);

    {
        auto expr = MakeOp(1033, OpType::Add, {a, MakeOp(1034, OpType::Mul, {a, b})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 2);
    }

    {
        auto expr = MakeOp(1035, OpType::Add, {MakeOp(1036, OpType::Shl, {a, MakeConst(1037, 1)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 2);
    }

    {
        auto expr = MakeOp(1038, OpType::Add, {MakeOp(1039, OpType::Mul, {a, b}), MakeOp(1040, OpType::Mul, {a, c})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        BF_TEST(result->inputs.size() == 2);
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestLinearMultiplicity_BasicAndMixed);
    BF_RUN_TEST(TestLinearMultiplicity_Guards);
    return 0;
}
