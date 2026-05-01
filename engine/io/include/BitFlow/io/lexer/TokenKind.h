#pragma once

namespace BitFlow::IO::Lexer {

enum class TokenKind {
    EndOfInput,
    Error,

    Identifier,
    DecimalLiteral,
    HexLiteral,

    LeftParen,
    RightParen,
    Comma,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    Ampersand, // &
    Pipe,      // |
    Caret,     // ^
    Tilde,     // ~

    ShiftLeft,  // <<
    ShiftRight, // >>
};

} // namespace BitFlow::IO::Lexer
