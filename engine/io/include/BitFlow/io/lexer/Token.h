#pragma once

#include <BitFlow/io/lexer/TokenKind.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace BitFlow::IO::Lexer {

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
    std::optional<std::uint64_t> numericValue;
};

} // namespace BitFlow::IO::Lexer
