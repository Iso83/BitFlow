# Core

## Interning / Canonicalization *(all nodes)*
### Deduplicate
$(A \oplus B) \equiv (B \oplus A) \to$ same instance<br>
👉 *AST::ExprIntern::Intern*


## Rules *(1 node)*
### Stage: Normalize
#### Flatten
$(X \oplus Y) \oplus Z \to X \oplus Y \oplus Z$<br>
$X \oplus (Y \oplus Z) \to X \oplus Y \oplus Z$
#### Ordering
*Canonical form: commutative operators*<br>
$X \oplus Y \oplus X \to sort \to X \oplus X \oplus Y \to cancel \to Y$

