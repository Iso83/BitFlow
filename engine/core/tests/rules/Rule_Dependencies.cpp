#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <unordered_set>
#include <vector>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

namespace {

using Getter = Rule (*)();

Rule GetRuleById(CoreRuleType id) {
    switch (id) {
    case CoreRuleType::Normalize_Flatten:
        return Normalize::Get_Flatten_Rule();
    case CoreRuleType::Normalize_Order:
        return Normalize::Get_Order_Rule();
    case CoreRuleType::Simplify_AddZero:
        return Simplify::Arithmetic::Get_Add_Zero_Rule();
    case CoreRuleType::Simplify_SubZero:
        return Simplify::Arithmetic::Get_Sub_Zero_Rule();
    case CoreRuleType::Simplify_MulZero:
        return Simplify::Arithmetic::Get_Mul_Zero_Rule();
    case CoreRuleType::Simplify_ModZeroGuard:
        return Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule();
    case CoreRuleType::Simplify_ShiftZero:
        return Simplify::Arithmetic::Get_Shift_Zero_Rule();
    case CoreRuleType::Simplify_RotateModuloBitwidth:
        return Simplify::Arithmetic::Get_Rotate_Zero_Rule();
    case CoreRuleType::Simplify_MulOne:
        return Simplify::Arithmetic::Get_Mul_One_Rule();
    case CoreRuleType::Simplify_DivOne:
        return Simplify::Arithmetic::Get_Div_One_Rule();
    case CoreRuleType::Simplify_NegNeg:
        return Simplify::Arithmetic::Get_Neg_Neg_Rule();
    case CoreRuleType::Simplify_XorZero:
        return Simplify::Bitwise::Get_Xor_Zero_Rule();
    case CoreRuleType::Simplify_AddFold:
        return Simplify::Arithmetic::Get_Add_Fold_Rule();
    case CoreRuleType::Simplify_ArithmeticConstCombine:
        return Simplify::Arithmetic::Get_Const_Combine_Rule();
    case CoreRuleType::Simplify_AndFold:
        return Simplify::Bitwise::Get_And_Fold_Rule();
    case CoreRuleType::Simplify_OrFold:
        return Simplify::Bitwise::Get_Or_Fold_Rule();
    case CoreRuleType::Simplify_XorFold:
        return Simplify::Bitwise::Get_Xor_Fold_Rule();
    case CoreRuleType::Simplify_XorCancel:
        return Simplify::Bitwise::Get_Xor_Cancel_Rule();
    case CoreRuleType::Simplify_AndCancel:
        return Simplify::Bitwise::Get_And_Cancel_Rule();
    case CoreRuleType::Simplify_OrCancel:
        return Simplify::Bitwise::Get_Or_Cancel_Rule();
    case CoreRuleType::Simplify_Not:
        return Simplify::Bitwise::Get_Not_Rule();
    case CoreRuleType::Simplify_NotPushdown:
        return Simplify::Bitwise::Get_Not_Pushdown_Rule();
    case CoreRuleType::Simplify_NotXor:
        return Simplify::Bitwise::Get_Not_Xor_Rule();
    case CoreRuleType::Simplify_Idempotent:
        return Simplify::Bitwise::Get_Idempotent_Rule();
    case CoreRuleType::Simplify_And_Idempotent:
        return Simplify::Bitwise::Get_And_Idempotent_Rule();
    case CoreRuleType::Simplify_Complement:
        return Simplify::Bitwise::Get_Complement_Rule();
    case CoreRuleType::Simplify_AndXorReduction:
        return Simplify::Bitwise::Get_And_Xor_Reduction_Rule();
    case CoreRuleType::Simplify_XorAndReduction:
        return Simplify::Bitwise::Get_Xor_And_Reduction_Rule();
    case CoreRuleType::Simplify_XorNotReduction:
        return Simplify::Bitwise::Get_Xor_Not_Reduction_Rule();
    case CoreRuleType::Simplify_AndZeroDominance:
        return Simplify::Bitwise::Get_And_Zero_Dominance_Rule();
    case CoreRuleType::Simplify_AndOneIdentity:
        return Simplify::Bitwise::Get_And_One_Identity_Rule();
    case CoreRuleType::Simplify_OrOneDominance:
        return Simplify::Bitwise::Get_Or_One_Dominance_Rule();
    case CoreRuleType::Simplify_OrZeroIdentity:
        return Simplify::Bitwise::Get_Or_Zero_Identity_Rule();
    case CoreRuleType::Simplify_CH:
        return Simplify::Get_CH_Simplify_Rule();
    case CoreRuleType::Simplify_MAJ:
        return Simplify::Get_MAJ_Simplify_Rule();
    case CoreRuleType::Factorize_XorAnd:
        return Factorize::Bitwise::Get_Xor_And_Rule();
    case CoreRuleType::Factorize_XorPairCancel:
        return Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule();
    case CoreRuleType::Factorize_AddLinearMultiplicity:
        return Factorize::Arithmetic::Get_Add_Linear_Multiplicity_Rule();
    case CoreRuleType::Factorize_AddCommonFactor:
        return Factorize::Arithmetic::Get_Add_CommonFactor_Rule();
    case CoreRuleType::Factorize_MulCombineConstants:
        return Factorize::Arithmetic::Get_Mul_CombineConstants_Rule();
    case CoreRuleType::Factorize_AndAbsorb:
        return Factorize::Bitwise::Get_And_Absorb_Rule();
    case CoreRuleType::Factorize_OrAbsorb:
        return Factorize::Bitwise::Get_Or_Absorb_Rule();
    case CoreRuleType::Factorize_Distribute:
        return Factorize::Bitwise::Get_Distribute_Rule();
    }

    return Normalize::Get_Flatten_Rule();
}

std::vector<Getter> AllRuleGetters() {
    return {
        Normalize::Get_Flatten_Rule,
        Normalize::Get_Order_Rule,
        Simplify::Arithmetic::Get_Add_Zero_Rule,
        Simplify::Arithmetic::Get_Sub_Zero_Rule,
        Simplify::Arithmetic::Get_Mul_Zero_Rule,
        Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule,
        Simplify::Arithmetic::Get_Shift_Zero_Rule,
        Simplify::Arithmetic::Get_Rotate_Zero_Rule,
        Simplify::Arithmetic::Get_Mul_One_Rule,
        Simplify::Arithmetic::Get_Div_One_Rule,
        Simplify::Arithmetic::Get_Neg_Neg_Rule,
        Simplify::Bitwise::Get_Xor_Zero_Rule,
        Simplify::Arithmetic::Get_Add_Fold_Rule,
        Simplify::Arithmetic::Get_Const_Combine_Rule,
        Simplify::Bitwise::Get_And_Fold_Rule,
        Simplify::Bitwise::Get_Or_Fold_Rule,
        Simplify::Bitwise::Get_Xor_Fold_Rule,
        Simplify::Bitwise::Get_Xor_Cancel_Rule,
        Simplify::Bitwise::Get_And_Cancel_Rule,
        Simplify::Bitwise::Get_Or_Cancel_Rule,
        Simplify::Bitwise::Get_Not_Rule,
        Simplify::Bitwise::Get_Not_Pushdown_Rule,
        Simplify::Bitwise::Get_Not_Xor_Rule,
        Simplify::Bitwise::Get_Idempotent_Rule,
        Simplify::Bitwise::Get_And_Idempotent_Rule,
        Simplify::Bitwise::Get_Complement_Rule,
        Simplify::Bitwise::Get_And_Xor_Reduction_Rule,
        Simplify::Bitwise::Get_Xor_And_Reduction_Rule,
        Simplify::Bitwise::Get_Xor_Not_Reduction_Rule,
        Simplify::Bitwise::Get_And_Zero_Dominance_Rule,
        Simplify::Bitwise::Get_And_One_Identity_Rule,
        Simplify::Bitwise::Get_Or_One_Dominance_Rule,
        Simplify::Bitwise::Get_Or_Zero_Identity_Rule,
        Simplify::Get_CH_Simplify_Rule,
        Simplify::Get_MAJ_Simplify_Rule,
        Factorize::Bitwise::Get_Xor_And_Rule,
        Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule,
        Factorize::Arithmetic::Get_Add_Linear_Multiplicity_Rule,
        Factorize::Arithmetic::Get_Add_CommonFactor_Rule,
        Factorize::Arithmetic::Get_Mul_CombineConstants_Rule,
        Factorize::Bitwise::Get_And_Absorb_Rule,
        Factorize::Bitwise::Get_Or_Absorb_Rule,
        Factorize::Bitwise::Get_Distribute_Rule,
    };
}

void AddRuleAndDeps(RuleEngine& engine, CoreRuleType id, std::unordered_set<uint32_t>& added) {
    const uint32_t numericId = static_cast<uint32_t>(id);
    if (added.find(numericId) != added.end())
        return;

    Rule rule = GetRuleById(id);
    for (uint32_t dep : rule.Dependencies) {
        AddRuleAndDeps(engine, static_cast<CoreRuleType>(dep), added);
    }

    engine.AddRule(rule);
    added.insert(rule.Id);
}

int Test_RuleDeps_NoDeps_Fails() {
    bool tested = false;
    for (Getter getter : AllRuleGetters()) {
        Rule rule = getter();
        if (rule.Dependencies.empty())
            continue;

        tested = true;
        RuleEngine engine;
        bool threw = false;
        try {
            engine.AddRule(rule);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        BF_TEST(threw);
    }

    BF_TEST(tested);
    return 0;
}

int Test_RuleDeps_WithDeps_Works() {
    bool tested = false;
    uint32_t seed = 9000;
    for (Getter getter : AllRuleGetters()) {
        Rule rule = getter();
        if (rule.Dependencies.empty())
            continue;

        tested = true;
        RuleEngine engine;
        std::unordered_set<uint32_t> added;
        AddRuleAndDeps(engine, rule.id, added);

        auto x = MakeVar(seed++);
        ExprOld* result = engine.Rewrite(x);
        BF_TEST(result->id == x->id);
    }

    BF_TEST(tested);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_RuleDeps_NoDeps_Fails);
    BF_RUN_TEST(Test_RuleDeps_WithDeps_Works);
    return 0;
}
