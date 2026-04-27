#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <unordered_set>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

namespace {

using Getter = Rule (*)();

Rule GetRuleById(RuleId id) {
    switch (id) {
    case RuleId::Normalize_Flatten:
        return Normalize::Get_Flatten_Rule();
    case RuleId::Normalize_Order:
        return Normalize::Get_Order_Rule();
    case RuleId::Simplify_AddZero:
        return Simplify::Arithmetic::Get_Add_Zero_Rule();
    case RuleId::Simplify_SubZero:
        return Simplify::Arithmetic::Get_Sub_Zero_Rule();
    case RuleId::Simplify_MulZero:
        return Simplify::Arithmetic::Get_Mul_Zero_Rule();
    case RuleId::Simplify_ModZeroGuard:
        return Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule();
    case RuleId::Simplify_ShiftZero:
        return Simplify::Arithmetic::Get_Shift_Zero_Rule();
    case RuleId::Simplify_RotateModuloBitwidth:
        return Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule();
    case RuleId::Simplify_MulOne:
        return Simplify::Arithmetic::Get_Mul_One_Rule();
    case RuleId::Simplify_DivOne:
        return Simplify::Arithmetic::Get_Div_One_Rule();
    case RuleId::Simplify_NegNeg:
        return Simplify::Arithmetic::Get_Neg_Neg_Rule();
    case RuleId::Simplify_XorZero:
        return Simplify::Bitwise::Get_Xor_Zero_Rule();
    case RuleId::Simplify_AddFold:
        return Simplify::Arithmetic::Get_Add_Fold_Rule();
    case RuleId::Simplify_ArithmeticConstCombine:
        return Simplify::Arithmetic::Get_Const_Combine_Rule();
    case RuleId::Simplify_AndFold:
        return Simplify::Bitwise::Get_And_Fold_Rule();
    case RuleId::Simplify_OrFold:
        return Simplify::Bitwise::Get_Or_Fold_Rule();
    case RuleId::Simplify_XorFold:
        return Simplify::Bitwise::Get_Xor_Fold_Rule();
    case RuleId::Simplify_XorCancel:
        return Simplify::Bitwise::Get_Xor_Cancel_Rule();
    case RuleId::Simplify_AndCancel:
        return Simplify::Bitwise::Get_And_Cancel_Rule();
    case RuleId::Simplify_OrCancel:
        return Simplify::Bitwise::Get_Or_Cancel_Rule();
    case RuleId::Simplify_Not:
        return Simplify::Bitwise::Get_Not_Rule();
    case RuleId::Simplify_NotPushdown:
        return Simplify::Bitwise::Get_NotPushdown_Rule();
    case RuleId::Simplify_NotXor:
        return Simplify::Bitwise::Get_Not_Xor_Rule();
    case RuleId::Simplify_Idempotent:
        return Simplify::Bitwise::Get_Idempotent_Rule();
    case RuleId::Simplify_And_Idempotent:
        return Simplify::Bitwise::Get_And_Idempotent_Rule();
    case RuleId::Simplify_Complement:
        return Simplify::Bitwise::Get_Complement_Rule();
    case RuleId::Simplify_AndXorReduction:
        return Simplify::Bitwise::Get_And_Xor_Reduction_Rule();
    case RuleId::Simplify_XorAndReduction:
        return Simplify::Bitwise::Get_Xor_And_Reduction_Rule();
    case RuleId::Simplify_XorNotReduction:
        return Simplify::Bitwise::Get_Xor_Not_Reduction_Rule();
    case RuleId::Simplify_AndZeroDominance:
        return Simplify::Bitwise::Get_And_ZeroDominance_Rule();
    case RuleId::Simplify_AndOneIdentity:
        return Simplify::Bitwise::Get_And_OneIdentity_Rule();
    case RuleId::Simplify_OrOneDominance:
        return Simplify::Bitwise::Get_Or_OneDominance_Rule();
    case RuleId::Simplify_OrZeroIdentity:
        return Simplify::Bitwise::Get_Or_ZeroIdentity_Rule();
    case RuleId::Simplify_CH:
        return Simplify::Get_CH_Simplify_Rule();
    case RuleId::Simplify_MAJ:
        return Simplify::Get_MAJ_Simplify_Rule();
    case RuleId::Factorize_XorAnd:
        return Factorize::Bitwise::Get_Xor_And_Rule();
    case RuleId::Factorize_XorPairCancel:
        return Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule();
    case RuleId::Factorize_AddLinearMultiplicity:
        return Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule();
    case RuleId::Factorize_AddCommonFactor:
        return Factorize::Arithmetic::Get_Add_CommonFactor_Rule();
    case RuleId::Factorize_MulCombineConstants:
        return Factorize::Arithmetic::Get_Mul_CombineConstants_Rule();
    case RuleId::Factorize_AndAbsorb:
        return Factorize::Bitwise::Get_And_Absorb_Rule();
    case RuleId::Factorize_OrAbsorb:
        return Factorize::Bitwise::Get_Or_Absorb_Rule();
    case RuleId::Factorize_Distribute:
        return Factorize::Bitwise::Get_Distribute_Rule();
    }

    return Normalize::Get_Flatten_Rule();
}

void AddRuleWithDeps(RuleEngine& engine, RuleId id, std::unordered_set<uint32_t>& added) {
    const uint32_t numericId = static_cast<uint32_t>(id);
    if (added.find(numericId) != added.end())
        return;

    Rule rule = GetRuleById(id);
    for (uint32_t dep : rule.Dependencies) {
        AddRuleWithDeps(engine, static_cast<RuleId>(dep), added);
    }

    engine.AddRule(rule);
    added.insert(rule.Id);
}

int RunRuleBasic(Getter getter, uint32_t seed) {
    RuleEngine engine;
    std::unordered_set<uint32_t> added;
    const Rule rule = getter();
    AddRuleWithDeps(engine, rule.id, added);

    auto x = MakeVar(seed);
    Expr* result = engine.Rewrite(x);
    BF_TEST(result->id == x->id);
    return 0;
}

#define RULE_LIST(X)                                                                                                   \
    X(Rule_Normalize_Flatten_Basic, Normalize::Get_Flatten_Rule)                                                       \
    X(Rule_Normalize_Order_Basic, Normalize::Get_Order_Rule)                                                           \
    X(Rule_Simplify_AddZero_Basic, Simplify::Arithmetic::Get_Add_Zero_Rule)                                            \
    X(Rule_Simplify_SubZero_Basic, Simplify::Arithmetic::Get_Sub_Zero_Rule)                                            \
    X(Rule_Simplify_MulZero_Basic, Simplify::Arithmetic::Get_Mul_Zero_Rule)                                            \
    X(Rule_Simplify_ModZeroGuard_Basic, Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule)                                 \
    X(Rule_Simplify_ShiftZero_Basic, Simplify::Arithmetic::Get_Shift_Zero_Rule)                                        \
    X(Rule_Simplify_RotateModuloBitwidth_Basic, Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule)                 \
    X(Rule_Simplify_MulOne_Basic, Simplify::Arithmetic::Get_Mul_One_Rule)                                              \
    X(Rule_Simplify_DivOne_Basic, Simplify::Arithmetic::Get_Div_One_Rule)                                              \
    X(Rule_Simplify_NegNeg_Basic, Simplify::Arithmetic::Get_Neg_Neg_Rule)                                              \
    X(Rule_Simplify_XorZero_Basic, Simplify::Bitwise::Get_Xor_Zero_Rule)                                               \
    X(Rule_Simplify_AddFold_Basic, Simplify::Arithmetic::Get_Add_Fold_Rule)                                            \
    X(Rule_Simplify_ArithmeticConstCombine_Basic, Simplify::Arithmetic::Get_Const_Combine_Rule)                        \
    X(Rule_Simplify_AndFold_Basic, Simplify::Bitwise::Get_And_Fold_Rule)                                               \
    X(Rule_Simplify_OrFold_Basic, Simplify::Bitwise::Get_Or_Fold_Rule)                                                 \
    X(Rule_Simplify_XorFold_Basic, Simplify::Bitwise::Get_Xor_Fold_Rule)                                               \
    X(Rule_Simplify_XorCancel_Basic, Simplify::Bitwise::Get_Xor_Cancel_Rule)                                           \
    X(Rule_Simplify_AndCancel_Basic, Simplify::Bitwise::Get_And_Cancel_Rule)                                           \
    X(Rule_Simplify_OrCancel_Basic, Simplify::Bitwise::Get_Or_Cancel_Rule)                                             \
    X(Rule_Simplify_Not_Basic, Simplify::Bitwise::Get_Not_Rule)                                                        \
    X(Rule_Simplify_NotPushdown_Basic, Simplify::Bitwise::Get_NotPushdown_Rule)                                        \
    X(Rule_Simplify_NotXor_Basic, Simplify::Bitwise::Get_Not_Xor_Rule)                                                 \
    X(Rule_Simplify_Idempotent_Basic, Simplify::Bitwise::Get_Idempotent_Rule)                                          \
    X(Rule_Simplify_And_Idempotent_Basic, Simplify::Bitwise::Get_And_Idempotent_Rule)                                  \
    X(Rule_Simplify_Complement_Basic, Simplify::Bitwise::Get_Complement_Rule)                                          \
    X(Rule_Simplify_AndXorReduction_Basic, Simplify::Bitwise::Get_And_Xor_Reduction_Rule)                              \
    X(Rule_Simplify_XorAndReduction_Basic, Simplify::Bitwise::Get_Xor_And_Reduction_Rule)                              \
    X(Rule_Simplify_XorNotReduction_Basic, Simplify::Bitwise::Get_Xor_Not_Reduction_Rule)                              \
    X(Rule_Simplify_AndZeroDominance_Basic, Simplify::Bitwise::Get_And_ZeroDominance_Rule)                             \
    X(Rule_Simplify_AndOneIdentity_Basic, Simplify::Bitwise::Get_And_OneIdentity_Rule)                                 \
    X(Rule_Simplify_OrOneDominance_Basic, Simplify::Bitwise::Get_Or_OneDominance_Rule)                                 \
    X(Rule_Simplify_OrZeroIdentity_Basic, Simplify::Bitwise::Get_Or_ZeroIdentity_Rule)                                 \
    X(Rule_Simplify_CH_Basic, Simplify::Get_CH_Simplify_Rule)                                                          \
    X(Rule_Simplify_MAJ_Basic, Simplify::Get_MAJ_Simplify_Rule)                                                        \
    X(Rule_Factorize_XorAnd_Basic, Factorize::Bitwise::Get_Xor_And_Rule)                                               \
    X(Rule_Factorize_XorPairCancel_Basic, Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule)                                \
    X(Rule_Factorize_AddLinearMultiplicity_Basic, Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule)              \
    X(Rule_Factorize_AddCommonFactor_Basic, Factorize::Arithmetic::Get_Add_CommonFactor_Rule)                          \
    X(Rule_Factorize_MulCombineConstants_Basic, Factorize::Arithmetic::Get_Mul_CombineConstants_Rule)                  \
    X(Rule_Factorize_AndAbsorb_Basic, Factorize::Bitwise::Get_And_Absorb_Rule)                                         \
    X(Rule_Factorize_OrAbsorb_Basic, Factorize::Bitwise::Get_Or_Absorb_Rule)                                           \
    X(Rule_Factorize_Distribute_Basic, Factorize::Bitwise::Get_Distribute_Rule)

#define DECLARE_RULE_TEST(name, getter)                                                                                \
    int name() {                                                                                                       \
        return RunRuleBasic(getter, __LINE__);                                                                         \
    }

RULE_LIST(DECLARE_RULE_TEST)

} // namespace

int main() {
#define RUN_RULE_TEST(name, getter) BF_RUN_TEST(name);
    RULE_LIST(RUN_RULE_TEST);
#undef RUN_RULE_TEST
    return 0;
}
