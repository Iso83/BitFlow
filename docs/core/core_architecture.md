# BitFlow: Core Architecture

## Info

### Ids
`ExprId` uniquely identifies every expression. Regardless of its actual value or graph structure, each expression is assigned a unique `ExprId` by the `ExprStore` at creation time.

An `Expr` itself only contains references (`ExprId`s) to its inputs. It does not know its own identity within the store.

---

### Expression
Every expression is identified by a unique ID managed by the `ExprStore`. The store is also responsible for releasing IDs when expressions are no longer needed.

`ExprRef` acts as a lightweight binding between an `ExprId` and its `ExprStore`. It is primarily intended for end-user usage, allowing expressions to be composed using operator overloading without excessive boilerplate.

`OpType` defines the operation of an expression and how it should be applied to its inputs.

For constants, `bitWidth` must always be respected. Large constants can be represented using multiple expressions.  
Example: a 128-bit value can be split into:
- `exprA` → bits 0–63  
- `inputs[0]` → continuation (bits 64–127)

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

Each rule has a unique `RuleKey` (fully qualified name). The `RuleEngine` uses this to ensure that a rule is only added once to the internal collection.



# Rules
## Normalize
- Flatten
- Order
## Simplify
### Arithmetic
- Add_Zero
- Mul_One
- Mul_Zero
- Sub_Zero
- Div_One
- Mod_Zero_Guard
- Shift_Zero
- Rotate_Zero
- Neg_Neg
- Add_Fold
- Const_Combine
### Bitwise
- Xor_Zero
- And_Fold
- Or_Fold
- Xor_Fold
- And_Cancel
- Or_Cancel
- Xor_Cancel
- Not
- Not_Pushdown
- Not_Xor
- Idempotent
- And_Idempotent
- Complement
- And_Xor_Reduction
- Xor_And_Reduction
- Xor_Not_Reduction
- And_Zero_Dominance
- And_One_Identity
- Or_One_Dominance
- Or_Zero_Identity
## Factorize
### Arithmetic
- Add_Linear_Multiplicity
- Add_CommonFactor
- Mul_CombineConstants
### Bitwise
- Xor_And
- Xor_Pair_Cancel
- And_Absorb
- Or_Absorb
- Distribute