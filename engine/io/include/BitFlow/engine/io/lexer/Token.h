#pragma once

#include <BitFlow/engine/core/types/Types.h>
#include <BitFlow/engine/io/lexer/TokenKind.h>
#include <optional>
#include <string>

namespace BitFlow::Engine::IO::Lexer {

enum class LexerErrorCode {
    UnexpectedCharacter,
    InvalidDecimalLiteral,
    MissingHexDigits,
    InvalidHexLiteral,
    MissingBinaryDigits,
    InvalidBinaryLiteral,
};

struct SourceSpan {
    std::size_t begin;
    std::size_t end;

    [[nodiscard]] constexpr std::size_t Length() const noexcept {
        return end - begin;
    }
};

struct Token {
    TokenKind kind;
    std::string text;
    SourceSpan span;
    std::optional<Core::Types::ExprChunk> numericValue;
    std::optional<LexerErrorCode> errorCode;
};

} // namespace BitFlow::IO::Lexer
