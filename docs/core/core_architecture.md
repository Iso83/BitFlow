
# BitFlow: Core Architecture
## Info
### Ids
`ExprId` uniquely identifies every expression. Regardless of its actual value or graph structure, each expression is assigned a unique `ExprId` by the `ExprStore` at creation time. 

An `Expr` itself only contains references (`ExprId`s) to its inputs. It does not know its own identity within the store.

`StrongId` is used throughout Core to provide strongly typed identifiers while still remaining lightweight and hashable for STL containers (`unordered_map`, `unordered_set`, etc.).

---
### Expression
Every expression is identified by a unique ID managed by the `ExprStore`. The store is also responsible for releasing IDs when expressions are no longer needed.

`ExprRef` acts as a lightweight binding between an `ExprId` and its `ExprStore`. It is primarily intended for end-user usage, allowing expressions to be composed using operator overloading without excessive boilerplate.

`OpType` defines the operation of an expression and how it should be applied to its inputs.

For constants, `bitWidth` must always be respected. Large constants can be represented using multiple expressions.

Example: a 128-bit value can be split into:
- `exprA` → bits 0–63
- `inputs[0]` → continuation (bits 64–127)

Expressions are immutable from the perspective of rewrite logic. Rewrites create new expressions instead of mutating existing nodes in-place.

`ExprStore` owns all expression memory and is responsible for allocating expression IDs and backing storage.

---
### Expr Lifetime & Debug Validation
The Core rewrite system relies heavily on temporary `Expr` references during traversal and optimization.

Because rewrites may create new expressions (`store->create(...)`), internal storage can reallocate, invalidating previously acquired references.

To help detect these cases, Core provides:
- `BF_EXPR_LIFETIME_CHECKS`
- `ExprDebug`
- field-level hooks (`FieldHook<T>`)
- checked input containers (`CheckedExprInputs`)
  
When enabled, every field access validates:
- expression ownership
- generation validity
- stale reference usage
- invalidated input access
  
This allows hidden UB cases to be detected immediately during CTest execution.
### Important Rewrite Rule
When a rewrite creates new expressions, expression data must first be snapshotted into stable value copies.

Correct pattern: 
```cpp
const Expr& e = (*store)[id];

const std::vector<ExprId> inputs = e.inputs;
const Types::BitWidth bitWidth = e.bitWidth;
```
Avoid:

```cpp
const auto inputs = e.inputs;
const auto bitWidth = e.bitWidth;
```
because hooked/proxy debug types may remain tied to the original `Expr`.

---
### Evaluator
`BitVector` represents a dynamic bit array and is used to evaluate expressions at runtime in C++. It is mainly intended for validation in CTest, ensuring that rules produce correct results.

The `Evaluator` resolves an expression into a concrete value (via `BitVector`).

To do this, all inputs must be constants. When variables are present, they can be resolved using randomized input (RNG-based evaluation).

---
### Helper
Attributes are used to mark classes or functions that are intended for cleanup or refactoring.

`ExprPrinter` (internal) is a minimal `ToString` utility used for debugging in CTest.
It exists in Core to keep `InfixInfo` consistent.
The public version is provided in BitFlow-IO, which also includes an `ExprParser`.

`ExprUtils` (internal) contains common helper functions for working with expressions.

---
## Rules
### Rule Model
A rule consists of:
- a unique `RuleKey`
- a `match(...)` function
- a `rewrite(...)` function
- optional dependencies

```cpp
struct Rule {
    RuleKey key;

    bool (*match)(const ExprStore*, ExprId);
    ExprId (*rewrite)(ExprStore*, ExprId);

    std::vector<RuleKey> deps{};
};
```

Rules operate on immutable expression graphs:
- `match(...)` checks whether a rewrite is applicable
- `rewrite(...)` returns either:
  - the original `ExprId`
  - or a newly created replacement expression
---
### RuleKey
Each rule uses a fully qualified `RuleKey`.

Example:
```txt
CORE.SIMPLIFY.BITWISE.XOR_CANCEL
```

The fully qualified naming system:
- keeps rule ownership explicit
- avoids collisions
- supports external rule packs
- improves rewrite tracing/debugging
- improves dependency diagnostics

---
### RuleEngine
`RuleEngine` applies rewrite rules recursively over expression trees.

Main responsibilities:
- recursive rewrite traversal
- dependency validation
- rewrite ordering
- rewrite tracing/debugging
- duplicate rule prevention

The engine validates:
- missing dependencies
- invalid dependency ordering
- redundant direct dependencies

Dependencies are expressed through `RuleKey`.

Example:

```cpp
Rule{
    Simplify::Bitwise::XorCancel,
    &Match_XorCancel,
    &Rewrite_XorCancel,
    { Normalize::Order }
};
```

---
### Rewrite Pipeline
Rewrites are generally executed in phases:
1. Normalize
2. Simplify
3. Factorize

Pipeline builders (`RulePipeline`) automatically include required dependencies.

Example:
```cpp
BuildNormalize()
BuildSimplifyArithmetic()
BuildSimplifyBitwise()
BuildFactorizeArithmetic()
BuildFactorizeBitwise()
```

---

# BitFlow Core Rules Reference

This document provides an overview of the built-in rewrite rules available in the BitFlow Core engine.
Rules are grouped by namespace and listed in the same order as the internal rule registry.

---

# Normalize

### CORE.NORMALIZE.FLATTEN

Flattens associative expressions into a single node structure.
This improves canonicalization and simplifies later rewrite matching.

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$(a + b) + c$$       |
| Rewrite | $$a + b + c$$         |

---

### CORE.NORMALIZE.ORDER

Sorts commutative inputs into a deterministic order.
This ensures structurally equivalent expressions share the same layout.

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$b + a$$             |
| Rewrite | $$a + b$$             |

---

# Normalize::Bitwise

### CORE.NORMALIZE.BITWISE.ROTATE_MODULO

Normalizes rotate amounts using the active bit width.

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$rotl(x, 32)$$       |
| Rewrite | $$rotl(x, 0)$$        |

---

# Simplify::Arithmetic

### CORE.SIMPLIFY.ARITHMETIC.ADD_ZERO

Removes additive zero terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + 0$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_ONE

Removes multiplicative identity terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 1$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_ZERO

Reduces multiplication by zero to zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 0$$ |
| Rewrite | $$0$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_ZERO

Removes subtraction by zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - 0$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.DIV_ONE

Removes division by one.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x / 1$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.MOD_ZERO_GUARD

Protects modulo operations against invalid simplifications involving zero divisors.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \bmod 0$$ |
| Rewrite | $$x \bmod 0$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SHIFT_ZERO

Removes shifts by zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x << 0$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.ROTATE_ZERO

Removes rotations by zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$rotl(x, 0)$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.NEG_NEG

Eliminates nested negation.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$-(-x)$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_NEG

Eliminates subtraction of a negated value.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - (-y)$$ |
| Rewrite | $$x + y$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.ADD_FOLD

Combines multiple constant terms inside additive expressions into a single constant value.
This helps normalize arithmetic expressions into a more stable canonical form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + 10 + 20$$ |
| Rewrite | $$x + 30$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_CONSTANT_FOLD

Moves subtraction of constant values into additive constant groups.
This simplifies arithmetic chains and improves canonicalization of affine expressions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(x + 8) - 1$$ |
| Rewrite | $$x + 7$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_ADD_SELF_CANCEL

Removes a matching term from an additive expression when it is immediately subtracted afterwards.
This simplifies expressions by eliminating redundant additive/subtractive pairs.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a + b + c) - b$$ |
| Rewrite | $$a + c$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_MUL_LINEAR_CANCEL

Reduces multiplicative linear terms when one matching base term is subtracted.
The rule decreases the multiplicative coefficient by one while preserving remaining factors.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 5 - x$$ |
| Rewrite | $$x \cdot 4$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_DIV_CONSTANT_REDUCTION

Reduces multiplicative constant factors before division when the division can be resolved exactly.
This simplifies arithmetic expressions by folding divisible constant coefficients inside multiplication chains.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 12 \div 3$$ |
| Rewrite | $$x \cdot 4$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_TO_POW

Rewrites repeated multiplicative factors into power expressions.
This reduces duplicated multiplication chains into a more compact exponential form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot x$$ |
| Rewrite | $$x^2$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.COMBINE_MUL_POW

Combines multiplicative power expressions with matching bases by adding their exponents.
This normalizes exponential multiplication chains into a single power expression.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot x^2$$ |
| Rewrite | $$x^3$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.COMBINE_CONSTANTS

Merges arithmetic constant chains into a reduced form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2 + 3 + 4$$ |
| Rewrite | $$9$$      |

---

# Simplify::Bitwise

### CORE.SIMPLIFY.BITWISE.XOR_ZERO

Removes XOR with zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus 0$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.BITWISE.AND_FOLD

Combines constant AND terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land 255 \land 15$$ |
| Rewrite | $$x \land 15$$ |

---

### CORE.SIMPLIFY.BITWISE.OR_FOLD

Combines constant OR terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor 1 \lor 2$$ |
| Rewrite | $$x \lor 3$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_FOLD

Combines constant XOR terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus 1 \oplus 1$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.AND_CANCEL

Cancels duplicate AND terms where possible.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land x$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.OR_CANCEL

Cancels duplicate OR terms where possible.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor x$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_CANCEL

Cancels duplicate XOR pairs.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus x$$ |
| Rewrite | $$0$$ |

---

### CORE.SIMPLIFY.BITWISE.NOT

Simplifies nested NOT operations.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\sim(\sim x)$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.NOT_PUSHDOWN

Pushes NOT operators deeper into expressions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\sim(x \land y)$$ |
| Rewrite | $$\sim x \lor \sim y$$ |

---

### CORE.SIMPLIFY.BITWISE.NOT_XOR

Normalizes NOT/XOR relationships.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\sim(x \oplus y)$$ |
| Rewrite | $$(\sim x) \oplus y$$ |

---

### CORE.SIMPLIFY.BITWISE.IDEMPOTENT

Simplifies idempotent bitwise patterns.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor x$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.AND_IDEMPOTENT

Simplifies repeated AND terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land x$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.COMPLEMENT

Simplifies complement patterns.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land \sim x$$ |
| Rewrite | $$0$$ |

---

### CORE.SIMPLIFY.BITWISE.AND_XOR_REDUCTION

Reduces mixed AND/XOR combinations into simpler forms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(x \oplus y) \land x$$ |
| Rewrite | $$x \land \sim y$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_AND_REDUCTION

Reduces XOR expressions involving masked terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus (x \land y)$$ |
| Rewrite | $$x \land \sim y$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_NOT_REDUCTION

Simplifies XOR expressions involving complements.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus \sim x$$ |
| Rewrite | $$-1$$ |

---

### CORE.SIMPLIFY.BITWISE.AND_ZERO_DOMINANCE

Applies AND dominance with zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land 0$$ |
| Rewrite | $$0$$ |

---

### CORE.SIMPLIFY.BITWISE.AND_ONE_IDENTITY

Removes all-ones masks where possible.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land -1$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.OR_ONE_DOMINANCE

Applies OR dominance with all-ones values.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor -1$$ |
| Rewrite | $$-1$$ |

---

### CORE.SIMPLIFY.BITWISE.OR_ZERO_IDENTITY

Removes OR with zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor 0$$ |
| Rewrite | $$x$$ |

---

# Factorize::Arithmetic

### CORE.FACTORIZE.ARITHMETIC.ADD_LINEAR_MULTIPLICITY

Converts repeated additive terms into multiplicative form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + x + x$$ |
| Rewrite | $$3 \cdot x$$ |

---

### CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_FACTOR

Extracts shared multiplicative factors from additions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a \cdot x + b \cdot x$$ |
| Rewrite | $$(a + b) \cdot x$$ |

---

### CORE.FACTORIZE.ARITHMETIC.COMMON_FACTOR_CANCEL_POW_TERMS

Cancels identical power terms that appear in both numerator and denominator.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a^5 \cdot 2}{3 \cdot a^5}$$ |
| Rewrite | $$\frac{2}{3}$$ |

---

### CORE.FACTORIZE.ARITHMETIC.MUL_COMBINE_CONSTANTS

Combines constant multiplication chains.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2 \cdot 3 \cdot x$$ |
| Rewrite | $$6 \cdot x$$ |

---

# Factorize::Bitwise

### CORE.FACTORIZE.BITWISE.XOR_AND

Extracts common XOR/AND structures into reduced forms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(x \land y) \oplus (x \land z)$$ |
| Rewrite | $$x \land (y \oplus z)$$ |

---

### CORE.FACTORIZE.BITWISE.XOR_PAIR_CANCEL

Cancels duplicated XOR factor pairs.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus y \oplus x$$ |
| Rewrite | $$y$$ |

---

### CORE.FACTORIZE.BITWISE.AND_ABSORB

Applies absorption rules for AND expressions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land (x \lor y)$$ |
| Rewrite | $$x$$ |

---

### CORE.FACTORIZE.BITWISE.OR_ABSORB

Applies absorption rules for OR expressions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor (x \land y)$$ |
| Rewrite | $$x$$ |

---

### CORE.FACTORIZE.BITWISE.DISTRIBUTE

Distributes bitwise expressions into expanded form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land (y \oplus z)$$ |
| Rewrite | $$(x \land y) \oplus (x \land z)$$ |
