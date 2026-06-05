# Operator Precedence

BitFlow follows the operator precedence rules of the C language.

Reference:
[Precedence and associativity of C operators](https://learn.microsoft.com/en-us/cpp/c-language/precedence-and-order-of-evaluation?view=msvc-170#precedence-and-associativity-of-c-operators)

## Supported Operators

From highest precedence to lowest precedence:

| Level | Operators             | Description     | Associativity |
| ----- | --------------------- | --------------- | ------------- |
| 0     | `( )`                 | Grouping        | Explicit      |
| 1     | `~`					| Unary operators | Right-to-left |
| 2     | `**`                  | Power           | Right-to-left |
| 3     | `*` `/` `%`           | Multiplicative  | Left-to-right |
| 4     | `+` `-`               | Additive        | Left-to-right |
| 5     | `<<` `>>` `<<<` `>>>` | Shift / Rotate  | Left-to-right |
| 6     | `&`                   | Bitwise AND     | Left-to-right |
| 7     | `^`                   | Bitwise XOR     | Left-to-right |
| 8     | `|`                   | Bitwise OR      | Left-to-right |

## Examples

```text id="r0z4i7"
a & (b | c)
```

is interpreted exactly as written because parentheses always take precedence.

```text id="f8cn9k"
a ^ b & c
```

is interpreted as:

```text id="v4snhl"
a ^ (b & c)
```

```text id="g7zaxv"
a | b ^ c
```

is interpreted as:

```text id="t0chx4"
a | (b ^ c)
```

```text id="j6xzpo"
a & b + c
```

is interpreted as:

```text id="v3xn7r"
a & (b + c)
```

```text id="r3hn8q"
a + b & c
```

is interpreted as:

```text id="y4nq2w"
(a + b) & c
```

```text id="yw3rfa"
a >>> 2 ^ a >>> 13 ^ a >>> 22
```

is interpreted as:

```text id="jlwm6g"
((a >>> 2) ^ (a >>> 13)) ^ (a >>> 22)
```
