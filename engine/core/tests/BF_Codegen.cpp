#include <BitFlow/core/codegen/Emitter.h>

#include <tests/common/Core_Expr.h>
#include <tests/common/TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(10, OpType::Add, {a, b});

    auto code = Codegen::EmitCExpr(expr, 32);

    BF_TEST(code == "((v1 + v2)) & ((1ull << 32) - 1)");
    return 0;
}
