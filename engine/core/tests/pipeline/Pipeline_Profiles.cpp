#include <BitFlow/core/ast/ExprStruct.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <stdexcept>
#include <string>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

namespace {

bool IsStable(RuleEngine& engine, Expr* expr) {
    Expr* next = engine.ApplyOnce(expr);
    return BitFlow::Core::AST::StructEqual(expr, next);
}

int Test_Pipeline_SHA_Safe() {
    auto a = MakeVar(1000);
    auto b = MakeVar(1001);
    auto c = MakeVar(1002);
    auto d = MakeVar(1003);
    auto e = MakeVar(1004);

    auto ch = MakeOp(1100, OpType::Ch, {a, b, c});
    auto maj = MakeOp(1101, OpType::Maj, {a, d, e});
    auto expr = MakeOp(1102, OpType::Xor, {ch, maj});

    RuleEngine engine = BuildProfile("sha_safe");
    Expr* out = engine.ApplyOnce(expr);

    BF_TEST(IsStable(engine, out));
    return 0;
}

int Test_Pipeline_NoOscillation_Factorize() {
    auto a = MakeVar(1200);
    auto b = MakeVar(1201);
    auto c = MakeVar(1202);

    auto andAB = MakeOp(1203, OpType::And, {a, b});
    auto andAC = MakeOp(1204, OpType::And, {a, c});
    auto expr = MakeOp(1205, OpType::Xor, {andAB, andAC});

    RuleEngine engine = BuildProfile("factorize");
    Expr* out = engine.ApplyOnce(expr);

    BF_TEST(IsStable(engine, out));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_Pipeline_SHA_Safe);
    BF_RUN_TEST(Test_Pipeline_NoOscillation_Factorize);
    return 0;
}
