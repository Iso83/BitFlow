# Core Rules-documentatie

Dit document beschrijft de huidige rule-engine in `engine/core`, inclusief:
- de **uitvoer-volgorde** (stages + pipeline-order),
- de **wiskundige transformaties** per rule,
- en hoe de engine regels toepast tot een vaste vorm (*stable form*).

---

## 1) Canonicalization buiten de rule-pipeline

### Interning / Deduplicatie (alle nodes)
Na rewrites wordt geïnternd zodat structureel gelijke expressies dezelfde instance/id krijgen.

Voorbeeld (commutatief equivalent):
- $(A \oplus B) \equiv (B \oplus A)$
- na canonicalization/intering kunnen beide naar dezelfde interned representatie wijzen.

---

## 2) Hoe rules toegepast worden (engine-gedrag)

De engine werkt in grote lijnen als volgt:
1. **Recursief naar kinderen** (post-order): eerst inputs herschrijven.
2. **ApplyOnce op huidige node**: rules van boven naar beneden in pipeline-volgorde aflopen.
3. Zodra een rule iets wijzigt, herstart de scan vanaf de eerste rule.
4. Herhaal tot geen enkele rule meer wijzigt.
5. **Intern** het eindresultaat.
6. `ApplyUntilStable` herhaalt dit over de boom tot pointer-identiteit stabiel blijft.

Belangrijk: in debug mag stage-volgorde niet teruglopen (anders exception op `AddRule`).

---

## 3) Stage-volgorde

De stage-enum is:
1. `Stage_Normalize`
2. `Stage_Simplify`
3. `Stage_Factorize`

Dit borgt: normaliseren $\rightarrow$ vereenvoudigen $\rightarrow$ factoriseren.<br>

---

## 4) Actieve pipeline-volgorde (huidige Add_Bitwise_Simplify_Pipeline)

Rules worden exact in deze volgorde toegevoegd:

### Stage Normalize
1. **Flatten**
2. **Order**

### Stage Simplify
3. **Xor_Cancel**
4. **Xor_Fold**
5. **Xor_Zero**
6. **And_Fold**
7. **Or_Fold**

### Stage Factorize
8. **Xor_And_CommonFactor**
9. **Xor_Pair_Cancel**

> Opmerking: `Add_Zero` bestaat als rule, maar zit momenteel niet in deze pipeline.

---

## 5) Rules met wiskundige stappen

## Stage Normalize

### 5.1 Flatten
Doel: geneste knopen met dezelfde operator 1 niveau omhoog trekken.

Voorbeelden:
- $(X \oplus Y) \oplus Z \rightarrow X \oplus Y \oplus Z$
- $X \oplus (Y \oplus Z) \rightarrow X \oplus Y \oplus Z$
- analoog voor andere operators met identieke parent/child-op.

Mechaniek:
- Voor elke input: als `input.op == parent.op` en niet-const met children, splice diens children in parent.

---

### 5.2 Order (alleen commutatieve ops)
Commutatieve operators: $+, \oplus, \land, \lor$.

Stap:
- sorteer inputs oplopend op expr-id.

Effect:
- canonical order, bv. $Y \oplus X \oplus X \rightarrow X \oplus X \oplus Y$
- hierdoor worden cancel/fold-matches later deterministischer.

---

## Stage Simplify

### 5.3 Xor_Cancel
Regel: gelijke termen in XOR heffen elkaar paarsgewijs op.

Wiskundig:
- $A \oplus A = 0$
- algemeen: termen met even multipliciteit verdwijnen; oneven multipliciteit blijft 1x staan.

Voorbeelden:
- $A \oplus B \oplus A \rightarrow B$
- $A \oplus A \rightarrow 0$
- $A \oplus A \oplus A \rightarrow A$

---

### 5.4 Xor_Fold
Regel: XOR alle constanten samen tot 1 constante accumulator.

Wiskundig:
- $(C_1 \oplus C_2 \oplus \dots \oplus C_n) = C_{acc}$
- expressie wordt: niet-constante termen $\oplus$ eventueel $C_{acc}$ (alleen als $C_{acc} \neq 0$).

Voorbeelden:
- $A \oplus 5 \oplus 3 \rightarrow A \oplus 6$
- $A \oplus 1 \oplus 1 \rightarrow A$
- $7 \oplus 7 \rightarrow 0$

---

### 5.5 Xor_Zero
Regel: verwijder neutraal element 0 uit XOR.

Wiskundig:
- $X \oplus 0 = X$

Voorbeelden:
- $A \oplus 0 \rightarrow A$
- $A \oplus B \oplus 0 \rightarrow A \oplus B$
- alleen nullen $0 \oplus 0 \rightarrow 0$

---

### 5.6 And_Fold
Regels voor AND met constanten:
- absorberend: $X \land 0 = 0$
- neutraal: $X \land 1 = X$

Voorbeelden:
- $A \land 0 \land B \rightarrow 0$
- $A \land 1 \land B \rightarrow A \land B$
- alleen enen $1 \land 1 \rightarrow 1$

---

### 5.7 Or_Fold
Regels voor OR met constanten:
- absorberend: $X \lor 1 = 1$
- neutraal: $X \lor 0 = X$

Voorbeelden:
- $A \lor 1 \lor B \rightarrow 1$
- $A \lor 0 \lor B \rightarrow A \lor B$
- alleen nullen $0 \lor 0 \rightarrow 0$

---

## Stage Factorize

### 5.8 Xor_And_CommonFactor
Patroon met 2 AND-termen onder XOR die een gemeenschappelijke factor delen:
- $(a \land b) \oplus (a \land c) \rightarrow a \land (b \oplus c)$

Ook varianten door operand-volgorde:
- $(b \land a) \oplus (c \land a) \rightarrow a \land (b \oplus c)$
- etc.

---

### 5.9 Xor_Pair_Cancel
Patroon met 2 XOR-termen onder XOR met 1 gedeelde term:
- $(a \oplus b) \oplus (a \oplus c) \rightarrow b \oplus c$

Afleiding:
- $a \oplus a = 0$
- daarna $0 \oplus b \oplus c = b \oplus c$.

---

## 6) Niet-actieve maar aanwezige rule

### Add_Zero (bestaat, niet in huidige pipeline)
Wiskundig:
- $X + 0 = X$

Gedrag gelijk aan `Rewrite_Remove_Zero`:
- verwijdert alle `0` inputs,
- bij leeg resultaat retourneert `0`,
- bij 1 term retourneert die term.

---

## 7) Praktische implicatie van de volgorde

Omdat Normalize eerst draait, worden vormen eerst vlak en geordend, waardoor Simplify-rules (zoals XOR-cancel) consistenter matchen. Factorize komt pas daarna, zodat patronen op al vereenvoudigde input werken.
