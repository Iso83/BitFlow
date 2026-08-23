#pragma once

namespace BitFlow::Engine::IO::Lexer {

enum class TokenKind {
    EndOfInput,
    Error,

    Identifier,
    DecimalLiteral,
    HexLiteral,
    BinaryLiteral,

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

    RotLeft,  // <<<
    RotRight, // >>>

    Pow // **
};

} // namespace BitFlow::IO::Lexer
