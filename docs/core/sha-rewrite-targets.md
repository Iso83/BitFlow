# SHA rewrite targets (stap 26.2)

Doel: voor SHA-fragmenten expliciet vastleggen naar welke vorm we nu *wel* willen rewriten,
zonder al een volledige SHA-256 pipeline af te dwingen.

## Scope in deze stap

We richten ons alleen op kleine bouwstenen en round-subexpressies:

1. `Ch(x,y,z)`
2. `Maj(x,y,z)`
3. kleine round-fragmenten rond `T1`/`T2`-onderdelen

## Canonical/rewrite-doelen

### Target A — CH-expansie

`Ch(x,y,z)` rewrite-doel:

- `Xor(And(x,y), And(Not(x), z))`
- met normale normalize-regels (`Flatten`, `Order`) op commutatieve knopen.

### Target B — MAJ-expansie

`Maj(x,y,z)` rewrite-doel:

- `Xor(And(x,y), And(x,z), And(y,z))`
- met normale normalize-regels (`Flatten`, `Order`) op commutatieve knopen.

### Target C — kleine round-fragmenten zonder high-level SHA-ops

Voor round-subexpressies (bijv. `Add(BigSigma1(e), Ch(e,f,g))` en
`Add(BigSigma0(a), Maj(a,b,c))`) is het minimale rewrite-doel nu:

- geen `Ch`/`Maj` meer aanwezig na `Add_Simplify_SHA_Rules`;
- fragment blijft in reguliere core-ops (`Add`, `Xor`, `And`, `Not`, `RotR`, ...);
- nog **geen** verplichting om alles volledig te unrollen/factorizen.

## Niet-doelen (bewust uitgesteld)

- Geen complete canonical vorm voor volledige `T1`/`T2` ketens met alle algebra.
- Geen globale optimalisatie-doelen voor volledige SHA-256 rounds.
- Geen verplichting op één unieke rotatie-volgorde voorbij bestaande order-regels.
