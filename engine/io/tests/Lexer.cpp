#include "TestAssert.h"

#include <BitFlow/engine/io/lexer/Lexer.h>
#include <BitFlow/engine/io/lexer/TokenKind.h>
#include <cstdint>
#include <string>
#include <vector>

using BitFlow::Engine::IO::Lexer::Token;
using BitFlow::Engine::IO::Lexer::TokenKind;

int TestLexer_AllTokenTypes() {
    const std::string input = "foo 12 0x2A 0b1010 ( ) , + - * / % & | ^ ~ ** <<< >>>";
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize(input);

    const std::vector<TokenKind> expected = {
        TokenKind::Identifier, TokenKind::DecimalLiteral, TokenKind::HexLiteral, TokenKind::BinaryLiteral,
        TokenKind::LeftParen,  TokenKind::RightParen,     TokenKind::Comma,      TokenKind::Plus,
        TokenKind::Minus,      TokenKind::Star,           TokenKind::Slash,      TokenKind::Percent,
        TokenKind::Ampersand,  TokenKind::Pipe,           TokenKind::Caret,      TokenKind::Tilde,
        TokenKind::Pow,        TokenKind::RotLeft,        TokenKind::RotRight,   TokenKind::EndOfInput};

    CPPTEST_ASSERT(tokens.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        CPPTEST_ASSERT(tokens[i].kind == expected[i]);

    CPPTEST_ASSERT(tokens[1].numericValue.has_value());
    CPPTEST_ASSERT(tokens[1].numericValue.value() == static_cast<std::uint64_t>(12));
    CPPTEST_ASSERT(tokens[2].numericValue.has_value());
    CPPTEST_ASSERT(tokens[2].numericValue.value() == static_cast<std::uint64_t>(42));
    CPPTEST_ASSERT(tokens[3].numericValue.has_value());
    CPPTEST_ASSERT(tokens[3].numericValue.value() == static_cast<std::uint64_t>(10));

    return 0;
}

int TestLexer_WhitespaceIsSkipped() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize(" \t\n  abc   ");

    CPPTEST_ASSERT(tokens.size() == 2);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[0].text == "abc");
    CPPTEST_ASSERT(tokens[0].span.begin == 5);
    CPPTEST_ASSERT(tokens[0].span.end == 8);
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::EndOfInput);
    CPPTEST_ASSERT(tokens[1].text.empty());
    CPPTEST_ASSERT(tokens[1].span.begin == 11);
    CPPTEST_ASSERT(tokens[1].span.end == 11);

    return 0;
}

int TestLexer_IdentifierRules() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("_ok123 1abc");

    CPPTEST_ASSERT(tokens.size() == 4);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[0].text == "_ok123");
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::DecimalLiteral);
    CPPTEST_ASSERT(tokens[1].text == "1");
    CPPTEST_ASSERT(tokens[2].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[2].text == "abc");
    CPPTEST_ASSERT(tokens[3].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_HexVariants() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("0x10 0Xff");

    CPPTEST_ASSERT(tokens.size() == 3);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::HexLiteral);
    CPPTEST_ASSERT(tokens[0].numericValue.value() == static_cast<std::uint64_t>(16));
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::HexLiteral);
    CPPTEST_ASSERT(tokens[1].numericValue.value() == static_cast<std::uint64_t>(255));
    CPPTEST_ASSERT(tokens[2].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_BinaryVariants() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("0b1 0B11111111");

    CPPTEST_ASSERT(tokens.size() == 3);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::BinaryLiteral);
    CPPTEST_ASSERT(tokens[0].numericValue.value() == static_cast<std::uint64_t>(1));
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::BinaryLiteral);
    CPPTEST_ASSERT(tokens[1].numericValue.value() == static_cast<std::uint64_t>(255));
    CPPTEST_ASSERT(tokens[2].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_ShiftOperatorsLongestMatch() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("<< >>");

    CPPTEST_ASSERT(tokens.size() == 3);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::ShiftLeft);
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::ShiftRight);
    CPPTEST_ASSERT(tokens[2].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_InvalidInputProducesErrorToken() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("0x @");

    CPPTEST_ASSERT(tokens.size() == 1);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::Error);
    CPPTEST_ASSERT(tokens[0].text == "Expected hex digits after 0x prefix at position 0");
    CPPTEST_ASSERT(tokens[0].errorCode.has_value());
    CPPTEST_ASSERT(tokens[0].errorCode.value() == BitFlow::Engine::IO::Lexer::LexerErrorCode::MissingHexDigits);
    CPPTEST_ASSERT(tokens[0].span.begin == 0);
    CPPTEST_ASSERT(tokens[0].span.end == 2);

    return 0;
}

int TestLexer_UnexpectedCharacterHasPosition() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("@");

    CPPTEST_ASSERT(tokens.size() == 1);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::Error);
    CPPTEST_ASSERT(tokens[0].text == "Unexpected character at position 0");
    CPPTEST_ASSERT(tokens[0].errorCode.has_value());
    CPPTEST_ASSERT(tokens[0].errorCode.value() == BitFlow::Engine::IO::Lexer::LexerErrorCode::UnexpectedCharacter);
    CPPTEST_ASSERT(tokens[0].span.begin == 0);
    CPPTEST_ASSERT(tokens[0].span.end == 1);

    return 0;
}

int TestLexer_ImplicitMultiplicationXBeforeIdentifierWithDigits() {
    const std::vector<Token> tokens = BitFlow::Engine::IO::Lexer::Tokenize("5xx2+3");

    CPPTEST_ASSERT(tokens.size() == 6);
    CPPTEST_ASSERT(tokens[0].kind == TokenKind::DecimalLiteral);
    CPPTEST_ASSERT(tokens[0].text == "5");
    CPPTEST_ASSERT(tokens[1].kind == TokenKind::Star);
    CPPTEST_ASSERT(tokens[1].text == "x");
    CPPTEST_ASSERT(tokens[2].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[2].text == "x2");
    CPPTEST_ASSERT(tokens[3].kind == TokenKind::Plus);
    CPPTEST_ASSERT(tokens[4].kind == TokenKind::DecimalLiteral);
    CPPTEST_ASSERT(tokens[4].text == "3");
    CPPTEST_ASSERT(tokens[5].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_WidthConstructorsRemainIdentifiers() {
    const auto tokens = BitFlow::Engine::IO::Lexer::Tokenize("u8(a) u32(x)");

    CPPTEST_ASSERT(tokens[0].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[0].text == "u8");

    CPPTEST_ASSERT(tokens[4].kind == TokenKind::Identifier);
    CPPTEST_ASSERT(tokens[4].text == "u32");

    return 0;
}

int main() {
    CPPTEST_RUN(TestLexer_AllTokenTypes);
    CPPTEST_RUN(TestLexer_WhitespaceIsSkipped);
    CPPTEST_RUN(TestLexer_IdentifierRules);
    CPPTEST_RUN(TestLexer_HexVariants);
    CPPTEST_RUN(TestLexer_BinaryVariants);
    CPPTEST_RUN(TestLexer_ShiftOperatorsLongestMatch);
    CPPTEST_RUN(TestLexer_InvalidInputProducesErrorToken);
    CPPTEST_RUN(TestLexer_UnexpectedCharacterHasPosition);
    CPPTEST_RUN(TestLexer_ImplicitMultiplicationXBeforeIdentifierWithDigits);
    CPPTEST_RUN(TestLexer_WidthConstructorsRemainIdentifiers);
    return 0;
}
