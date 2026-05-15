
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
# Rewrite Transformations
## Normalize
### CORE.NORMALIZE.FLATTEN
Flattens associative operations into a single node.

| Step    | Expression      |
| ------- | --------------- |
| Input   | $$a + (b + c)$$ |
| Rewrite | $$a + b + c$$   |

---
### CORE.NORMALIZE.ORDER
Canonical input ordering for deterministic graph structure.

| Step    | Expression     |
| ------- | -------------- |
| Input   | $$b \oplus a$$ |
| Rewrite | $$a \oplus b$$ |

---
## Normalize.Bitwise
### CORE.NORMALIZE.BITWISE.ROTATE_MODULO
Normalizes rotate amounts modulo bit-width.

| Step    | Expression               |
| ------- | ------------------------ |
| Input   | $$\mathrm{rotl}(x, 32)$$ |
| Rewrite | $$\mathrm{rotl}(x, 0)$$  |

---
# Simplify
## Simplify.Arithmetic
### CORE.SIMPLIFY.ARITHMETIC.ADD_ZERO

| Step    | Expression |
| ------- | ---------- |
| Input   | $$x + 0$$  |
| Rewrite | $$x$$      |

---
### CORE.SIMPLIFY.ARITHMETIC.MUL_ONE

| Step    | Expression    |
| ------- | ------------- |
| Input   | $$x \cdot 1$$ |
| Rewrite | $$x$$         |

---
### CORE.SIMPLIFY.ARITHMETIC.MUL_ZERO

| Step    | Expression    |
| ------- | ------------- |
| Input   | $$x \cdot 0$$ |
| Rewrite | $$0$$         |

---
### CORE.SIMPLIFY.ARITHMETIC.NEG_NEG

| Step    | Expression |
| ------- | ---------- |
| Input   | $$-(-x)$$  |
| Rewrite | $$x$$      |

---
### CORE.SIMPLIFY.ARITHMETIC.COMBINE_CONSTANTS
Combines constants inside arithmetic chains.

| Step    | Expression    |
| ------- | ------------- |
| Input   | $$x + 2 + 3$$ |
| Rewrite | $$x + 5$$     |

---
## Simplify.Bitwise
### CORE.SIMPLIFY.BITWISE.XOR_CANCEL

| Step | Expression |
|---|---|
| Input | $$x \oplus x$$ |
| Rewrite | $$0$$ |

---
### CORE.SIMPLIFY.BITWISE.COMPLEMENT

| Step    | Expression         |
| ------- | ------------------ |
| Input   | $$x \land \neg x$$ |
| Rewrite | $$0$$              |

| Step    | Expression        |
| ------- | ----------------- |
| Input   | $$x \lor \neg x$$ |
| Rewrite | $$\mathrm{true}$$ |

---
### CORE.SIMPLIFY.BITWISE.AND_XOR_REDUCTION
Transforms AND/XOR reduction patterns into a reduced canonical form.

| Step    | Expression               |
| ------- | ------------------------ |
| Input   | $$x \land (x \oplus y)$$ |
| Rewrite | $$x \land \neg y$$       |
#### Notes
- Requires normalized operand ordering
- Commonly triggered after `CORE.NORMALIZE.ORDER`
- Reduces XOR dependency chains

---
### CORE.SIMPLIFY.BITWISE.XOR_AND_REDUCTION

| Step    | Expression               |
| ------- | ------------------------ |
| Input   | $$x \oplus (x \land y)$$ |
| Rewrite | $$x \land \neg y$$       |

---
# Factorize
## Factorize.Arithmetic
### CORE.FACTORIZE.ARITHMETIC.ADD_LINEAR_MULTIPLICITY
Combines repeated additive linear terms.

| Step    | Expression    |
| ------- | ------------- |
| Input   | $$a + a$$     |
| Rewrite | $$a \cdot 2$$ |

| Step    | Expression                    |
| ------- | ----------------------------- |
| Input   | $$a + a \cdot 2 + a \cdot 3$$ |
| Rewrite | $$a \cdot 6$$                 |

---
### CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_FACTOR
Extracts common multiplicative factors.

| Step    | Expression                    |
| ------- | ----------------------------- |
| Input   | $$(a \cdot b) + (a \cdot c)$$ |
| Rewrite | $$a \cdot (b + c)$$           |

---
### CORE.FACTORIZE.ARITHMETIC.MUL_COMBINE_CONSTANTS
Combines multiplicative constants.

| Step    | Expression            |
| ------- | --------------------- |
| Input   | $$x \cdot 2 \cdot 3$$ |
| Rewrite | $$x \cdot 6$$         |

---
## Factorize.Bitwise
### CORE.FACTORIZE.BITWISE.AND_ABSORB

| Step    | Expression             |
| ------- | ---------------------- |
| Input   | $$x \land (x \lor y)$$ |
| Rewrite | $$x$$                  |

---
### CORE.FACTORIZE.BITWISE.OR_ABSORB

| Step    | Expression             |
| ------- | ---------------------- |
| Input   | $$x \lor (x \land y)$$ |
| Rewrite | $$x$$                  |

---
### CORE.FACTORIZE.BITWISE.XOR_PAIR_CANCEL

| Step    | Expression                       |
| ------- | -------------------------------- |
| Input   | $$a \oplus b \oplus a \oplus b$$ |
| Rewrite | $$0$$                            |
