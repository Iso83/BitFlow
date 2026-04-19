# Engine-capabilities buiten rules (op basis van CTest)

> Vraag: “wat is er naast rules beschikbaar in deze engine?”
>
> Onderstaande inventaris komt uit de huidige CTest-targets en geeft dus een praktisch beeld van wat effectief gebruikt/getest wordt.

---

## 1) CTest referentietabel

| CTest target | Domein | Wat het aantoont |
|---|---|---|
| `BF_AST_OpTypeHelpers` | AST | Operator metadata/helpers aanwezig |
| `BF_Dedup` | Expression interning | Deduplicatie/hash-consing gedrag |
| `BF_ExprIntern_KeyBuilder` | Expression keys | Canonical key-opbouw voor interning |
| `BF_RuleEngine_InPlaceRewrite` | Rule engine | Rewrite-loop + stabilisatie |
| `BF_ConstantEval` | Eval | Constante evaluatie (bitvector-semantiek) |
| `BF_ConstantDetect` | Eval | Detectie of subtree constant is |
| `BF_Codegen` | Codegen | C-expression emit/codegen pad |
| `BF_SsaBuilder` | SSA | Opbouw SSA-programma uit Expr |
| `BF_PerfPass` | Codegen/opt | Performance/transform pass infrastructuur |
| `BF_Arithmetic_Integration` | Rules integration | End-to-end arithmetic normalize/simplify/factorize |
| `BF_Rules_Order` | Normalize | Canonieke operand-volgorde |
| `BF_Rules_Flatten` | Normalize | Associatieve flattening |
| `BF_Simplify_Bitwise_*` | Bitwise simplify | Fold/cancel/not/complement/dominance/etc. |
| `BF_Factorize_Bitwise_*` | Bitwise factorize | Common-factor/absorb/distribute/pair-cancel |
| `BF_Simplify_Arithmetic_*` | Arithmetic simplify | Zero/One/Fold/Const combine |
| `BF_Rules_CH` / `BF_Rules_MAJ` | SHA simplify | CH/MAJ specifieke simplificatie |
| `BF_ExprParser` | IO | Parser van tekst naar Expr AST |
| `BF_ExprPrinter` | IO | Pretty-printer Expr -> tekst |
| `BF_Lexer` | IO | Tokenizer/lexer infrastructuur |

---

## 2) Wat is dus aanwezig naast rules?

### A) Expression infrastructuur

- AST model (`Expr`, `OpType`)
- interning + dedup (hash-consing)
- key-builders en hashing voor structurele identiteit

### B) Evaluatie-laag

- `ConstantDetect` (is subtree constant?)
- `ConstantEval` (evaluatie met bitwidth-semantiek)

### C) Codegeneratie-laag

- C-expression emission
- SSA builder
- perf/codegen passes

### D) IO-laag

- lexer
- parser
- printer

### E) Test-harnas

- geïntegreerde module-tests per subsystem
- integratietests voor gecombineerde rule-pipelines

---

## 3) Waarom CTest als bron?

CTest toont welke subsystemen effectief “levend” zijn in de huidige build en regressie-afgedekt worden.
Daardoor is dit een praktische inventaris van engine-capabilities, niet alleen theoretische code-aanwezigheid.

---

## 4) Reproduceerbare inventaris-commando

```bash
BF_ENABLE_FETCH=FALSE ctest --test-dir build -N
```

