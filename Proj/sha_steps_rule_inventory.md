# SHA steps: rule inventory from legacy `sha steps`

Deze nota zet de belangrijkste algebra/logic regels uit het oude project (`Proj/sha steps`) in 1 plaats.
Ze zijn bedoeld als **checker/doelregels** voor de demo-app in `Proj`, zonder wijzigingen in `engine/core`.

## Prioritaire regels (reeds gebruikt in demo-flow)

- SHA functies:
  - `CH(x,y,z) = (x & y) ^ (~x & z)`
  - `MAJ(x,y,z) = (x & y) ^ (x & z) ^ (y & z)`
- Sigma functies:
  - `EP0(x) = ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22)`
  - `EP1(x) = ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25)`
  - `SIG0(x) = ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3)`
  - `SIG1(x) = ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10)`
- Schedule:
  - `W[i] = SIG1(W[i-2]) + W[i-7] + SIG0(W[i-15]) + W[i-16]`

## Checker-regels uit legacy `OrderOfOperations.cpp`

- De Morgan:
  - `a & b == ~(~a | ~b)`
  - `a | b == ~(~a & ~b)`
- XOR equivalenties:
  - `a ^ b == (~a & b) | (a & ~b)`
  - `a ^ b == (a | b) & ~(a & b)`
- CH alternatieve vormen:
  - `CH(a,b,c) == (~a | b) & (a | c)`
  - `CH(a,b,c) == (a & b) | (~a & c)`
- Distributieve bracket checks (waar geldig voor bitwise operators):
  - `a & (b | c) == (a & b) | (a & c)`
  - `a | (b & c) == (a | b) & (a | c)`

## Nog op te nemen als extra checker in Proj (geen engine/core wijziging)

1. Associativiteit/commutativiteit suites voor `+`, `^`, `&`, `|` op kleine testdomeinen.
2. Guard-regels rond shifts/rotates (`shift by 0`, `rotate modulo bitwidth`).
3. Extra CH/MAJ equivalentie-checks per stap zodat regressies snel zichtbaar zijn in demo-runs.
4. Expliciete detectie voor oscillatie (factorize/distribute heen-en-weer) met warning per profiel.

## Scope-afspraak

Nieuwe rule-checkers of experimentele rule-sets blijven in `Proj/**`.
Geen toevoegingen/wijzigingen onder `BitFlow-engine/core/**` voor deze opdracht.
