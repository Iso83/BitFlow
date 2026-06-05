
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

const ExprInputs inputs = e.inputs;
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
Descriptions below reflect the actual `match(...)` and `rewrite(...)` behavior in rule implementations, with extra examples derived from existing Core CTests where available.

---


# Normalize

### CORE.NORMALIZE.FLATTEN

Flattens associative operators (`+`, `*`, `AND`, `OR`, `XOR`) by pulling nested nodes with the same operator into one input list.
This is a canonicalization prerequisite for many later rules that depend on scanning siblings in a single node.
It does not reorder inputs by itself (ordering is handled by `CORE.NORMALIZE.ORDER`).

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$(a + b) + c$$       |
| Rewrite | $$a + b + c$$         |

---

### CORE.NORMALIZE.ORDER

Sorts inputs of commutative operators into a deterministic canonical order.
This guarantees structurally equivalent expressions serialize identically, improving cache hits and making pair-cancel/folding rules predictable.

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$b + a$$             |
| Rewrite | $$a + b$$             |

---

# Normalize::Arithmetic

### CORE.NORMALIZE.ARITHMETIC.ADD_NEG_TO_SUB

Converts additions containing a negated term into subtraction form.
This normalizes equivalent signed expressions into a more compact and consistent structure, enabling downstream simplification and canonicalization rules.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + (-y)$$ |
| Rewrite | $$x - y$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$1 + (-a)$$ |
| Rewrite | $$1 - a$$ |

---

### CORE.NORMALIZE.ARITHMETIC.SUB_TO_NEG

Rewrites subtraction into a negated canonical form when the left side is a smaller constant expression.
This normalizes equivalent signed expressions so structurally identical forms can later simplify or cancel.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$1 - a$$ |
| Rewrite | $$-(a - 1)$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2 - b$$ |
| Rewrite | $$-(b - 2)$$ |

---

# Normalize::Bitwise

### CORE.NORMALIZE.BITWISE.ROTATE_MODULO

Normalizes rotate amounts modulo the expression bit-width.
This keeps rotations canonical (`rot(x, w)` becomes `rot(x, 0)` for width `w`) and enables downstream `ROTATE_ZERO` elimination.

**Assuming a 32-bit expression width**

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$rotl(x, 32)$$       |
| Rewrite | $$x$$                 |

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$rotr(x, 40)$$       |
| Rewrite | $$rotr(x, 8)$$        |

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

| Step    | Expression |
| ------- | ---------- |
| Input   | $$1 \cdot 1 \cdot x$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.POW_ONE

Removes exponent identity terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x^1$$ |
| Rewrite | $$x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\left(a+b\right)^1$$ |
| Rewrite | $$a+b$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_ZERO

Reduces multiplication by zero to zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$0 \cdot x$$ |
| Rewrite | $$0$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.POW_ZERO

Replaces non-zero powers with the multiplicative identity.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x^0$$ |
| Rewrite | $$1$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\left(a+b\right)^0$$ |
| Rewrite | $$1$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_ZERO

Removes subtraction by zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - 0$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_SELF

Removes subtraction of an expression from itself.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - x$$  |
| Rewrite | $$0$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.DIV_ONE

Removes division by one.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x / 1$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.DIV_SELF

Removes division of an expression by itself.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x / x$$  |
| Rewrite | $$1$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.MOD_ONE

Removes modulo by one.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \bmod 1$$ |
| Rewrite | $$0$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.MOD_SELF

Removes modulo of an expression by itself.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \bmod x$$ |
| Rewrite | $$0$$      |

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

### CORE.SIMPLIFY.ARITHMETIC.SHIFT_ROTATE_CONSTANT_FOLD

Evaluates shifts and rotations when both the value and amount are constants.
The result keeps the bit width of the left-hand constant value.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$rotl(u8(129), 1)$$ |
| Rewrite | $$3$$      |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$u8(128) >> 7$$ |
| Rewrite | $$1$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.NEG_NEG

Eliminates nested negation.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$-(-x)$$  |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.ARITHMETIC.NEG_POW_EVEN

Removes negation from bases raised to an even exponent.
Since even powers eliminate sign changes, equivalent expressions normalize into a common canonical form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(-x)^2$$ |
| Rewrite | $$x^2$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(-(a-1))^2$$ |
| Rewrite | $$(a-1)^2$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(-x)^8$$ |
| Rewrite | $$x^8$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_NEG

Eliminates subtraction of a negated value.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - (-y)$$ |
| Rewrite | $$x + y$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x - (-(y+z))$$ |
| Rewrite | $$x + y + z$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.ADD_FOLD

Combines multiple constant terms inside additive expressions into a single constant value.
This helps normalize arithmetic expressions into a more stable canonical form.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + 10 + 20$$ |
| Rewrite | $$30 + x$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_CONSTANT_FOLD

Moves subtraction of constant values into additive constant groups.
This simplifies arithmetic chains and improves canonicalization of affine expressions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(x + 8) - 1$$ |
| Rewrite | $$7 + x$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_ADD_SELF_CANCEL

Cancels matching additive terms across subtraction.
This simplifies expressions by removing terms that appear on both sides of a subtraction, including terms nested inside additive or subtractive chains.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a + b + c) - b$$ |
| Rewrite | $$a + c$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a + b) - (b - 2)$$ |
| Rewrite | $$a + 2$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(1 + b) - (b - 2)$$ |
| Rewrite | $$3$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.SUB_MUL_LINEAR_CANCEL

Reduces multiplicative linear terms when one matching base term is subtracted.
The rule decreases the multiplicative coefficient by one while preserving remaining factors.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 5 - x$$ |
| Rewrite | $$4 \cdot x$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.MUL_DIV_CONSTANT_REDUCTION

Reduces multiplicative constant factors before division when the division can be resolved exactly.
This simplifies arithmetic expressions by folding divisible constant coefficients inside multiplication chains.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \cdot 12 \div 3$$ |
| Rewrite | $$4 \cdot x$$ |

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

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x^a \cdot x$$ |
| Rewrite | $$x^{a+1}$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x^a \cdot x^b$$ |
| Rewrite | $$x^{a+b}$$ |

---

### CORE.SIMPLIFY.ARITHMETIC.COMBINE_CONSTANTS

Evaluates constant-only arithmetic subexpressions (`+`, `-`, `*`, `/`, `%`, shifts/rotates where applicable) inside the current bit-width domain.
This is one of the key cleanup rules that collapses constant islands into a single literal and improves matchability of other simplifications.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2 + 3 + 4$$ |
| Rewrite | $$9$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$20 \div 4$$ |
| Rewrite | $$5$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$20 \cdot 5$$ |
| Rewrite | $$100$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$20 \bmod 6$$ |
| Rewrite | $$2$$ |

---

# Simplify::Bitwise

### CORE.SIMPLIFY.BITWISE.XOR_ZERO

Removes XOR with zero.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$0 \oplus x$$ |
| Rewrite | $$x$$      |

---

### CORE.SIMPLIFY.BITWISE.AND_FOLD

Simplifies AND expressions containing constant operands by bitwise-AND folding all constant terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land \text{true} \land 15$$ |
| Rewrite | $$15 \land x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land 0$$ |
| Rewrite | $$0$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$255 \land 15$$ |
| Rewrite | $$15$$ |

---

### CORE.SIMPLIFY.BITWISE.OR_FOLD

Simplifies OR expressions containing constant operands by bitwise-OR folding all constant terms. When duplicate OR terms are present, `OR_CANCEL` runs before this rule so constants can fold after idempotent operands are removed.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor 1 \lor 2$$ |
| Rewrite | $$3 \lor x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$1 \lor 2 \lor a \lor a$$ |
| Rewrite | $$3 \lor a$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor 0$$ |
| Rewrite | $$x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor \text{true}$$ |
| Rewrite | $$\text{true}$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_FOLD

Combines constant XOR terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus 1 \oplus 2$$ |
| Rewrite | $$3 \oplus x$$ |

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
| Rewrite | $$\text{true} \oplus x \oplus y$$ |

---

### CORE.SIMPLIFY.BITWISE.IDEMPOTENT

Simplifies idempotent bitwise patterns.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor x$$ |
| Rewrite | $$x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land x$$ |
| Rewrite | $$x$$ |

---

### CORE.SIMPLIFY.BITWISE.COMPLEMENT

Simplifies complement patterns by resolving expressions that contain both a value and its complement.
For bit-vectors this applies as `x & ~x = 0` and `x | ~x = all-ones` (width-aware full mask).

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land \sim x$$ |
| Rewrite | $$0$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \lor \sim x$$ |
| Rewrite | $$true$$ |

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

| Step    | Expression |
| ------- | ---------- |
| Input   | $$y \oplus (y \land x)$$ |
| Rewrite | $$y \land \sim x$$ |

---

### CORE.SIMPLIFY.BITWISE.XOR_AND_NOT_REDUCTION

Removes XOR operands that are masked by their own complement.

If an expression contains both `a` inside an XOR term and `~a` as an AND factor,
the `a` contribution can never affect the result and is removed from the XOR.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a \oplus b) \land \sim a$$ |
| Rewrite | $$b \land \sim a$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\sim a \land (a \oplus b \oplus c)$$ |
| Rewrite | $$\sim a \land (b \oplus c)$$ |

---

# Factorize::Arithmetic

### CORE.FACTORIZE.ARITHMETIC.ADD_LINEAR_MULTIPLICITY

Combines repeated linear terms by merging their coefficients into a single multiplicative expression.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + x + x$$ |
| Rewrite | $$3 \cdot x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2x + x$$ |
| Rewrite | $$3x$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + (2x - 4)$$ |
| Rewrite | $$3x - 4$$ |

---

### CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_FACTOR

Extracts shared multiplicative factors from additions.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a \cdot x + a \cdot y$$ |
| Rewrite | $$a \cdot (x+y)$$ |

---

### CORE.FACTORIZE.ARITHMETIC.PERFECT_SQUARE

Recognizes perfect-square trinomials and rewrites them as squared binomials.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a^2 + 2ab + b^2$$ |
| Rewrite | $$(a+b)^2$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a^2 - 2ab + b^2$$ |
| Rewrite | $$(a-b)^2$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a^2 - 6a + 9$$ |
| Rewrite | $$(a-3)^2$$ |

---

### CORE.FACTORIZE.ARITHMETIC.DIFFERENCE_OF_SQUARES

Factorizes a difference of two squared expressions into the product of their sum and difference.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a^2 - b^2$$ |
| Rewrite | $$(a+b)(a-b)$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a+1)^2 - (a-2)^2$$ |
| Rewrite | $$(1 + a + (a - 2)) \cdot (1 + a - (a - 2))$$ |

---


### CORE.FACTORIZE.ARITHMETIC.PROMOTE_FACTORS_TO_POWER

Promotes a complete set of plain multiplicative factors into an existing power whose base is their product.
This compacts expressions where one additional occurrence of a compound product appears next to a power of that same product.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a \cdot b \cdot c \cdot (a \cdot b)^2$$ |
| Rewrite | $$c \cdot (a \cdot b)^3$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a \cdot b \cdot c \cdot d \cdot (a \cdot b \cdot c)^5$$ |
| Rewrite | $$d \cdot (a \cdot b \cdot c)^6$$ |

---

### CORE.FACTORIZE.ARITHMETIC.COMMON_FACTOR_CANCEL_POW_TERMS

Cancels identical power terms and reduces power exponents that appear in both numerator and denominator.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a^5 \cdot 2}{3 \cdot a^5}$$ |
| Rewrite | $$\frac{2}{3}$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a^8 \cdot 2}{3 \cdot a^5}$$ |
| Rewrite | $$\frac{2 \cdot a^3}{3}$$ |

---

### CORE.FACTORIZE.ARITHMETIC.COMMON_FACTOR_CANCEL

Removes factors that appear on both sides of a division.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a \cdot b \cdot 2}{b \cdot 3}$$ |
| Rewrite | $$\frac{2 \cdot a}{3}$$ |

---

### CORE.FACTORIZE.ARITHMETIC.SUB_COMMON_DENOMINATOR

Extracts a shared denominator from subtraction expressions, including denominators embedded inside additive chains.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a}{c} - \frac{b}{c}$$ |
| Rewrite | $$\frac{a-b}{c}$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$40 + \frac{5}{8} - \frac{3}{8}$$ |
| Rewrite | $$40 + \frac{5-3}{8}$$ |

---

### CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_DENOMINATOR

Combines fractions with equal denominators into a single fraction.
This is primarily a structural normalization step that prepares arithmetic cancellation and constant folding passes.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$a / x + b / x$$ |
| Rewrite | $$(a + b) / x$$ |

---

### CORE.FACTORIZE.ARITHMETIC.MUL_FRACTION_NUMERATOR

Pushes multiplicative terms into a fraction numerator.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$2 \cdot \frac{3}{8}$$ |
| Rewrite | $$\frac{2 \cdot 3}{8}$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a}{b} \cdot c$$ |
| Rewrite | $$\frac{a \cdot c}{b}$$ |

---

### CORE.FACTORIZE.ARITHMETIC.DIV_FRACTION_DENOMINATOR

Eliminates division by a fraction by moving the inner denominator into the numerator.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$\frac{a}{\frac{b}{c}}$$ |
| Rewrite | $$\frac{a \cdot c}{b}$$ |

---

# Factorize::Bitwise

### CORE.FACTORIZE.BITWISE.XOR_AND

Extracts common XOR/AND structures into reduced forms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(y \land x) \oplus (z \land x)$$ |
| Rewrite | $$x \land (y \oplus z)$$ |

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \oplus (x \land y) \oplus (x \land z)$$ |
| Rewrite | $$x \land (z \oplus \lnot y)$$ |

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

Distributes bitwise forms when expansion improves downstream factoring/cancellation opportunities.
Although mathematically inverse of factoring rules, pipeline ordering determines when expansion is preferable for canonical shape.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x \land (y \oplus z)$$ |
| Rewrite | $$(x \land y) \oplus (x \land z)$$ |

---

### CORE.FACTORIZE.BITWISE.DISTRIBUTE_AND_OVER_OR

Factors a shared bitwise AND operand out of OR terms by applying the inverse of AND-over-OR distribution.
This rewrites OR expressions such as `(a & b) | (a & c)` into a single AND with the shared factor and an OR of the remaining terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a \land b) \lor (a \land c)$$ |
| Rewrite | $$a \land (b \lor c)$$ |

---

### CORE.FACTORIZE.BITWISE.DISTRIBUTE_OR_OVER_AND

Factors a shared bitwise OR operand out of AND terms by applying the inverse of OR-over-AND distribution.
This rewrites AND expressions such as `(a | b) & (a | c)` into a single OR with the shared factor and an AND of the remaining terms.

| Step    | Expression |
| ------- | ---------- |
| Input   | $$(a \lor b) \land (a \lor c)$$ |
| Rewrite | $$a \lor (b \land c)$$ |

---
