# Rules Reference (Core Engine)

> Doel: snel zien **welke rules bestaan**, in welke **vaste volgorde** ze draaien, welke **dependencies** ze hebben, en welke **wiskundige bewerking** ze representeren.

---

## 1) Stage-model en uitvoerorde

De engine hanteert stage-ordering:

1. `Stage_Normalize`
2. `Stage_Simplify_Pushdown`
3. `Stage_Simplify`
4. `Stage_Factorize`

Binnen een stage geldt insertion-order van de pipeline-functies in `RulePipeline.h`.

---

## 2) Snelle ref-tabel (alle pipeline-rules)

| # | RuleId | Builder | Stage | Pipeline | Dependencies (expliciet) | Wiskundige term |
|---:|---|---|---|---|---|---|
| 1 | `Normalize_Flatten` | `Get_Flatten_Rule` | Normalize | Normalize | – | Associativiteit normalisatie |
| 2 | `Normalize_Order` | `Get_Order_Rule` | Normalize | Normalize | – | Commutatieve canonieke ordening |
| 3 | `Simplify_NotPushdown` | `Get_NotPushdown_Rule` | Simplify_Pushdown | Simplify Bitwise | – | De Morgan / NOT-distributie |
| 4 | `Simplify_Not` | `Get_Not_Rule` | Simplify | Simplify Bitwise | – | Involutie/complement op constante |
| 5 | `Simplify_NotXor` | `Get_Not_Xor_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | XOR met NOT-herleiding |
| 6 | `Simplify_XorCancel` | `Get_Xor_Cancel_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Zelf-inverse (
`a ⊕ a = 0`) |
| 7 | `Simplify_AndCancel` | `Get_And_Cancel_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Contradictie/eliminatiepatroon |
| 8 | `Simplify_OrCancel` | `Get_Or_Cancel_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Tautologie/eliminatiepatroon |
| 9 | `Simplify_AndXorReduction` | `Get_And_Xor_Reduction_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Booleaanse reductie |
| 10 | `Simplify_XorNotReduction` | `Get_Xor_Not_Reduction_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Booleaanse reductie |
| 11 | `Simplify_XorAndReduction` | `Get_Xor_And_Reduction_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Booleaanse reductie |
| 12 | `Simplify_XorFold` | `Get_Xor_Fold_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Constant folding |
| 13 | `Simplify_AndFold` | `Get_And_Fold_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Constant folding |
| 14 | `Simplify_OrFold` | `Get_Or_Fold_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Constant folding |
| 15 | `Simplify_XorZero` | `Get_Xor_Zero_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Neutraal element |
| 16 | `Simplify_Idempotent` | `Get_Idempotent_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Idempotentie |
| 17 | `Simplify_And_Idempotent` | `Get_And_Idempotent_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Idempotentie |
| 18 | `Simplify_Complement` | `Get_Complement_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Complement-wet |
| 19 | `Simplify_AndZeroDominance` | `Get_And_ZeroDominance_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Dominantie |
| 20 | `Simplify_AndOneIdentity` | `Get_And_OneIdentity_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Identiteit |
| 21 | `Simplify_OrOneDominance` | `Get_Or_OneDominance_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Dominantie |
| 22 | `Simplify_OrZeroIdentity` | `Get_Or_ZeroIdentity_Rule` | Simplify | Simplify Bitwise | `Normalize_Flatten` | Identiteit |
| 23 | `Simplify_AddZero` | `Get_Add_Zero_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Additieve identiteit |
| 24 | `Simplify_SubZero` | `Get_Sub_Zero_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Subtractieve identiteit |
| 25 | `Simplify_MulOne` | `Get_Mul_One_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Multiplicatieve identiteit |
| 26 | `Simplify_MulZero` | `Get_Mul_Zero_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Annihilator |
| 27 | `Simplify_DivOne` | `Get_Div_One_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Identiteit deling |
| 28 | `Simplify_ModZeroGuard` | `Get_Mod_Zero_Guard_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Domein/guard-regel |
| 29 | `Simplify_NegNeg` | `Get_Neg_Neg_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Dubbele negatie |
| 30 | `Simplify_AddFold` | `Get_Add_Fold_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Constant folding |
| 31 | `Simplify_ArithmeticConstCombine` | `Get_Const_Combine_Rule` | Simplify | Simplify Arithmetic | `Normalize_Flatten` | Constant folding |
| 32 | `Factorize_XorAnd` | `Get_Xor_And_Rule` | Factorize | Factorize Bitwise | `Normalize_Flatten`,`Normalize_Order` | Distributiviteit omgekeerd |
| 33 | `Factorize_XorPairCancel` | `Get_Xor_Pair_Cancel_Rule` | Factorize | Factorize Bitwise | `Normalize_Flatten`,`Normalize_Order` | Pair cancellation |
| 34 | `Factorize_AndAbsorb` | `Get_And_Absorb_Rule` | Factorize | Factorize Bitwise | `Normalize_Flatten`,`Normalize_Order` | Absorptiewet |
| 35 | `Factorize_OrAbsorb` | `Get_Or_Absorb_Rule` | Factorize | Factorize Bitwise | `Normalize_Flatten`,`Normalize_Order` | Absorptiewet |
| 36 | `Factorize_Distribute` | `Get_Distribute_Rule` | Factorize | Factorize Bitwise | `Normalize_Flatten`,`Normalize_Order` | Distributiviteit |
| 37 | `Factorize_AddCommonFactor` | `Get_Add_CommonFactor_Rule` | Factorize | Factorize Arithmetic | `Normalize_Flatten`,`Normalize_Order` | Gemeenschappelijke factor / termcount |
| 38 | `Factorize_MulCombineConstants` | `Get_Mul_CombineConstants_Rule` | Factorize | Factorize Arithmetic | `Factorize_AddCommonFactor`,`Normalize_Flatten`,`Normalize_Order` | Coëfficiënt-combinatie |
| 39 | `Simplify_CH` | `Get_CH_Simplify_Rule` | Simplify | Simplify SHA (optioneel) | `Normalize_Flatten` | Keuzefunctie CH-optimalisatie |
| 40 | `Simplify_MAJ` | `Get_MAJ_Simplify_Rule` | Simplify | Simplify SHA (optioneel) | `Normalize_Flatten` | Majority MAJ-optimalisatie |

---

## 3) Huidige arithmetic focus (Stap 37 + 38)

### 3.1 Add common-factor (`Factorize_AddCommonFactor`)

Ondersteunt n-ary `Add` en doet:

- repeated-term counting: `t + t + t -> 3*t`
- common-factor extractie: `x*y + x*z -> x*(y+z)`

### 3.2 Mul constant combine (`Factorize_MulCombineConstants`)

Post-factorize coëfficiënt-combinatie in `Mul` met meerdere constanten:

- `3*(a*2)` (na flatten: `Mul(3,a,2)`) -> `6*a`

Dit geeft samen de gewenste keten:

- `a*2 + a*2 + a*2`
- `-> 3*(a*2)`
- `-> 6*a`

---

## 4) Dependency-regels in praktijk

`RuleEngine::AddRule` valideert dependencies strikt:

- een rule mag alleen toegevoegd worden als al haar `deps` al aanwezig zijn.
- stage mag in debug niet “terugvallen” (stage regression).

Dus voor nieuwe rules moet zowel dependency-set als insertion-order correct zijn.

---

## 5) Snelle checklist bij nieuwe rule

1. `RuleId` toevoegen.
2. Declaratie in `Rule.h` namespace.
3. Implementatie (`match/rewrite/stage/deps`).
4. Registratie in juiste pipeline in correcte volgorde.
5. Test toevoegen/uitbreiden in `engine/core/tests`.

