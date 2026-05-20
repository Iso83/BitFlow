# BitFlow

BitFlow is a symbolic bit-vector expression engine written in modern C++.

The project focuses on:
- expression graph rewriting
- algebraic simplification
- canonicalization
- symbolic evaluation
- bitwise and arithmetic transformations
- rule-based optimization pipelines

BitFlow started years ago as an older hobby project and has since been rebuilt from scratch with a cleaner architecture and modern tooling.  
The current version was heavily redesigned with help from modern AI-assisted tooling and focuses more on expression-level transformations rather than extremely low-level bit manipulation.

## Current Status

BitFlow is still under active development and does **not** yet have a stable `v1.0` release.

The core architecture is mostly stabilized, but several important systems are still intentionally simple:
- rewrite rules currently operate mainly on localized 2-node style transformations
- the `ExprStore` currently has no garbage collection or node compaction system
- large integer support is still limited to single integer chunk storage (no multi-array big integer backend yet)

Despite this, the rewrite engine and test infrastructure are already functional and heavily validated.

All rewrite rules are accompanied by dedicated CTests and are expected to pass before integration.

## Features

- Expr graph architecture
- Bit-width aware expressions
- Rule engine with dependency validation
- Canonicalization and normalization pipelines
- Constant folding
- Arithmetic and bitwise simplification
- Factorization pipelines
- Debug rewrite tracing
- Expression parser and printer
- LaTeX export
- Extensive CTest coverage

## Example

```cpp
auto x = V("x");
auto expr = (x ^ x) | 0;

auto simplified = Rewrite(engine, expr);

// => 0
```

## Documentation

Core architecture documentation can be found here:
- [core_architecture.md](docs/core/core_architecture.md)

A separate web-based demo and Docker deployment project is available here:
- [BitFlow.DotNet](https://github.com/Iso83/BitFlow.DotNet)

## Goals

The long-term goal of BitFlow is to provide a clean and extensible symbolic transformation engine suitable for:
- compiler experimentation
- optimization research
- symbolic simplification
- bit-vector analysis

Future plans include:
- advanced rewrite systems
- improved expression storage and memory management
- larger integer backends

## Build

BitFlow currently uses:
- CMake
- C++20
- CTest

## License

MIT License
