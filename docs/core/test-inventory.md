# Core test/build inventory (stap 21.4)

Deze inventaris noteert welke tests in `engine/core/tests` al in de tree staan, maar (bewust) niet standaard actief zijn in CMake.

## 1) Bewust niet in CMake opgenomen wegens ontbrekende API

- `BF_CodegenMulti.cpp`
- `BF_CodegenStructuralCSE.cpp`

Reden:
- Beide tests gebruiken `Codegen::EmitCFunctionMulti(...)`.
- De huidige publieke API in `engine/core/include/BitFlow/core/codegen/Emitter.h` expose’t enkel:
  - `EmitCExpr(...)`
  - `EmitCFunction(...)`
- Zonder `EmitCFunctionMulti(...)` in de actuele codebasis compileren deze tests niet.

Status:
- testbestanden blijven aanwezig als staged coverage voor toekomstige multi-output codegen,
- ze worden pas aan CMake gekoppeld zodra de API effectief beschikbaar is.

## 2) Bewust optioneel in CMake (wel compileerbaar)

- `BF_CodegenRuntime.cpp`
- `BF_FuzzEvalVsCodegen.cpp`

Reden:
- Deze draaien externe compiler/runtime stappen en zijn traag/toolchain-afhankelijk.
- Daarom hangen ze achter `BF_ENABLE_CODEGEN_RUNTIME_TESTS`.

## 3) Coherentie van de actieve testset

De actieve set bevat alleen tests die met de huidige publieke API en standaard buildopties compileerbaar en stabiel uitvoerbaar zijn.
