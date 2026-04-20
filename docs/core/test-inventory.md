# Core test/build inventory (stap 25.3)

Deze inventaris noteert het onderscheid tussen snelle unit-tests, tragere runtime-compile tests en fuzz/property tests.

## 1) Snelle unit tests (standaard actief)

- Alle reguliere `bf_add_module_test(...)` tests in `engine/core/CMakeLists.txt` en `engine/io/CMakeLists.txt` draaien standaard met label `unit`.
- De ExprWorkbench profile test (`BF_ExprWorkbench_StageProfiles`) heeft labels `unit;tool`.

Voorbeeld:
- `ctest -L unit`

## 2) Trage runtime compile tests (optioneel)

- `BF_CodegenRuntime.cpp` draait alleen als `BF_ENABLE_RUNTIME_COMPILE_TESTS=ON`.
- Label: `runtime_compile`.

Voorbeeld:
- `cmake -S . -B build -DBF_ENABLE_RUNTIME_COMPILE_TESTS=ON`
- `ctest -L runtime_compile`

## 3) Fuzz/property tests (optioneel)

- `BF_FuzzEvalVsCodegen.cpp` draait alleen als `BF_ENABLE_FUZZ_PROPERTY_TESTS=ON`.
- Labels: `fuzz;property`.

Voorbeeld:
- `cmake -S . -B build -DBF_ENABLE_FUZZ_PROPERTY_TESTS=ON`
- `ctest -L fuzz`
- `ctest -L property`

## 4) Backward compatibility

- Bestaande vlag `BF_ENABLE_CODEGEN_RUNTIME_TESTS` blijft beschikbaar als legacy switch en zet intern beide opties aan:
  - `BF_ENABLE_RUNTIME_COMPILE_TESTS=ON`
  - `BF_ENABLE_FUZZ_PROPERTY_TESTS=ON`
