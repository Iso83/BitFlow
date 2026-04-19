# Core rules en evaluatie (huidige codebasis)

Deze pagina beschrijft **alleen wat vandaag effectief in de code zit** in `engine/core`:
- stages en rule-ordering,
- actieve pipelines (`RulePipeline.h`),
- aanwezige arithmetic/bitwise/CH/MAJ rules,
- factorize-paden,
- constant evaluator-semantiek.

## 1. Rule engine gedrag

`RuleEngine` werkt als volgt:
1. `ApplyRecursive`: herschrijf eerst kinderen (post-order).
2. `ApplyOnce`: loop rules in toegevoegde volgorde.
3. Bij eerste succesvolle rewrite: herstart rule-scan vanaf rule 1.
4. Interning gebeurt op output (`ExprIntern::Intern`).
5. `ApplyUntilStable` herhaalt tot pointer-identiteit stabiel blijft.

Daarnaast valideert `AddRule`:
- geen duplicate `RuleId`,
- geldige rule,
- dependency-ids (`deps`) moeten al aanwezig zijn,
- in debug geen stage-regressie.

## 2. Huidige stages

Volgens `RuleStage.h`:
1. `Stage_Normalize` (0)
2. `Stage_Simplify_Pushdown` (1)
3. `Stage_Simplify` (2)
4. `Stage_Factorize` (3)

Praktisch: `Simplify_NotPushdown` draait in de pushdown-stage; de rest van simplify in `Stage_Simplify`.

## 3. Actieve pipelines in `RulePipeline.h`

### 3.1 Normalize
`Add_Normalize_Rules`:
- `Normalize_Flatten`
- `Normalize_Order`

### 3.2 Simplify (bitwise)
`Add_Simplify_Bitwise_Rules` voegt toe (in deze volgorde):
- NOT: `Simplify_NotPushdown`, `Simplify_Not`, `Simplify_NotXor`
- Cancel: `Simplify_XorCancel`, `Simplify_AndCancel`, `Simplify_OrCancel`
- Reductions: `Simplify_AndXorReduction`, `Simplify_XorNotReduction`, `Simplify_XorAndReduction`
- Fold: `Simplify_XorFold`, `Simplify_AndFold`, `Simplify_OrFold`
- Neutral/structure: `Simplify_XorZero`, `Simplify_Idempotent`, `Simplify_And_Idempotent`
- Logical/dominance: `Simplify_Complement`, `Simplify_AndZeroDominance`, `Simplify_AndOneIdentity`, `Simplify_OrOneDominance`, `Simplify_OrZeroIdentity`

### 3.3 Simplify (arithmetic)
`Add_Simplify_Arithmetic_Rules`:
- `Simplify_AddZero`
- `Simplify_SubZero`
- `Simplify_MulOne`
- `Simplify_MulZero`
- `Simplify_DivOne`
- `Simplify_ModZeroGuard`
- `Simplify_NegNeg`
- `Simplify_AddFold`
- `Simplify_ArithmeticConstCombine`

> Let op: `Simplify_ShiftZero` en `Simplify_RotateModuloBitwidth` bestaan als rule factories en `RuleId`, maar worden momenteel **niet** toegevoegd in `Add_Simplify_Arithmetic_Rules`.

### 3.4 Simplify (SHA)
`Add_Simplify_SHA_Rules` (optioneel, aparte pipeline call):
- `Simplify_CH`
- `Simplify_MAJ`

### 3.5 Factorize (bitwise)
`Add_Factorize_Bitwise_Rules`:
- `Factorize_XorAnd`
- `Factorize_XorPairCancel`
- `Factorize_AndAbsorb`
- `Factorize_OrAbsorb`
- `Factorize_Distribute`

### 3.6 Factorize (arithmetic)
`Add_Factorize_Arithmetic_Rules`:
- `Factorize_AddCommonFactor`
- `Factorize_MulCombineConstants`

## 4. Dependency-paden (relevant voor pipeline-combinaties)

Voorbeelden van harde dependencies in rules:
- veel simplify-rules vereisen `Normalize_Flatten`.
- `Factorize_XorPairCancel` vereist `Normalize_Flatten` + `Simplify_XorCancel`.
- `Factorize_XorAnd` en `Factorize_AddCommonFactor` vereisen `Normalize_Flatten` + `Normalize_Order`.
- `Factorize_MulCombineConstants` vereist `Factorize_AddCommonFactor` + normalize-dependencies.

Conclusie: factorize-profielen moeten in praktijk normalize (en deels simplify) beschikbaar maken.

## 5. CH/MAJ status

- `Simplify_CH` en `Simplify_MAJ` bestaan en worden gebruikt via `Add_Simplify_SHA_Rules`.
- Ze zitten **niet** in de standaard bitwise/arithmetic simplify pipeline; alleen wanneer die SHA-pipeline expliciet wordt toegevoegd.

## 6. Evaluator-semantiek (`EvaluateConstant`)

`EvaluateConstant(const Expr* root, uint32_t bitWidth)`:
- ondersteunt bitwidth `1..64`; anders `InvalidBitWidth`.
- werkt op `uint64_t` met maskering per stap (`mask = 2^w - 1`, of all-ones bij `w=64`).
- `Var` geeft `NotConstant`.
- ongeldige arity/op geeft `UnsupportedOp`.
- `Div` met 0 => `DivisionByZero`; `Mod` met 0 => `ModuloByZero`.
- `Neg` gebruikt two’s-complement (`~x + 1`) binnen mask.
- `Shl/Shr/UShr/RotL/RotR` normaliseren shift amount via `amount % bitWidth`.
- `Shr` en `UShr` zijn in evaluator effectief dezelfde logische right shift op unsigned data.
- `Ch` en `Maj` worden expliciet geëvalueerd met de standaard bitwise formules.

## 7. Scope-opmerking

Deze pagina documenteert alleen de huidige, gecompileerde en aangeroepen rule/eval onderdelen in `engine/core`.
Geen roadmap, geen wishlist, geen toekomstige semantiek.
