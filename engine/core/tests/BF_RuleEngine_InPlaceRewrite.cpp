#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <algorithm>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static bool Match_Xor_Unsorted(const Expr& e) {
    if (e.op != OpType::Xor || e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i - 1]->id.value() > e.inputs[i]->id.value())
            return true;
    }

    return false;
}

static Expr* Rewrite_Xor_InPlaceSort(Expr& e) {
    std::sort(e.inputs.begin(), e.inputs.end(), [](Expr* a, Expr* b) { return a->id.value() < b->id.value(); });
    return &e;
}

int TestEngineHandlesInPlaceRewriteSafely() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto e1 = MakeOp(100, OpType::Xor, {y, x});
    auto e2 = MakeOp(101, OpType::Xor, {y, x});

    RuleEngine engine;
    engine.AddRule(Rule{RuleId::Normalize_Order,
                        &Match_Xor_Unsorted,
                        &Rewrite_Xor_InPlaceSort,
                        0,
                        {},
                        RuleFlags::None,
                        "Normalize_Order"});

    Expr* r1 = engine.ApplyUntilStable(e1);
    Expr* r2 = engine.ApplyUntilStable(e2);

    BF_TEST(r1->op == OpType::Xor);
    BF_TEST(r1->inputs.size() == 2);
    BF_TEST(r1->inputs[0] == x);
    BF_TEST(r1->inputs[1] == y);
    BF_TEST(r1 == r2);
    return 0;
}

int main() {
    BF_RUN_TEST(TestEngineHandlesInPlaceRewriteSafely);
    return 0;
}
