#include <BitFlow/io/lexer/Lexer.h>
#include <BitFlow/io/lexer/TokenKind.h>
#include <TestAssert.h>
#include <cstdint>
#include <string>
#include <vector>

using BitFlow::IO::Lexer::Token;
using BitFlow::IO::Lexer::TokenKind;

namespace {

int TestLexer_AllTokenTypes() {
    const std::string input = "foo 12 0x2A ( ) , + - * / % & | ^ ~";
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize(input);

    const std::vector<TokenKind> expected = {
        TokenKind::Identifier, TokenKind::DecimalLiteral, TokenKind::HexLiteral, TokenKind::LeftParen,
        TokenKind::RightParen, TokenKind::Comma,          TokenKind::Plus,       TokenKind::Minus,
        TokenKind::Star,       TokenKind::Slash,          TokenKind::Percent,    TokenKind::Ampersand,
        TokenKind::Pipe,       TokenKind::Caret,          TokenKind::Tilde,      TokenKind::EndOfInput,
    };

    BF_TEST(tokens.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        BF_TEST(tokens[i].kind == expected[i]);

    BF_TEST(tokens[1].numericValue.has_value());
    BF_TEST(tokens[1].numericValue.value() == static_cast<std::uint64_t>(12));
    BF_TEST(tokens[2].numericValue.has_value());
    BF_TEST(tokens[2].numericValue.value() == static_cast<std::uint64_t>(42));

    return 0;
}

int TestLexer_WhitespaceIsSkipped() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize(" \t\n  abc   ");

    BF_TEST(tokens.size() == 2);
    BF_TEST(tokens[0].kind == TokenKind::Identifier);
    BF_TEST(tokens[0].text == "abc");
    BF_TEST(tokens[0].span.begin == 5);
    BF_TEST(tokens[0].span.end == 8);
    BF_TEST(tokens[1].kind == TokenKind::EndOfInput);
    BF_TEST(tokens[1].text.empty());
    BF_TEST(tokens[1].span.begin == 11);
    BF_TEST(tokens[1].span.end == 11);

    return 0;
}

int TestLexer_IdentifierRules() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize("_ok123 1abc");

    BF_TEST(tokens.size() == 4);
    BF_TEST(tokens[0].kind == TokenKind::Identifier);
    BF_TEST(tokens[0].text == "_ok123");
    BF_TEST(tokens[1].kind == TokenKind::DecimalLiteral);
    BF_TEST(tokens[1].text == "1");
    BF_TEST(tokens[2].kind == TokenKind::Identifier);
    BF_TEST(tokens[2].text == "abc");
    BF_TEST(tokens[3].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_HexVariants() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize("0x10 0Xff");

    BF_TEST(tokens.size() == 3);
    BF_TEST(tokens[0].kind == TokenKind::HexLiteral);
    BF_TEST(tokens[0].numericValue.value() == static_cast<std::uint64_t>(16));
    BF_TEST(tokens[1].kind == TokenKind::HexLiteral);
    BF_TEST(tokens[1].numericValue.value() == static_cast<std::uint64_t>(255));
    BF_TEST(tokens[2].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_ShiftOperatorsLongestMatch() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize("<< >>");

    BF_TEST(tokens.size() == 3);
    BF_TEST(tokens[0].kind == TokenKind::ShiftLeft);
    BF_TEST(tokens[1].kind == TokenKind::ShiftRight);
    BF_TEST(tokens[2].kind == TokenKind::EndOfInput);

    return 0;
}

int TestLexer_InvalidInputProducesErrorToken() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize("0x @");

    BF_TEST(tokens.size() == 1);
    BF_TEST(tokens[0].kind == TokenKind::Error);
    BF_TEST(tokens[0].text == "Expected hex digits after 0x prefix at position 0");
    BF_TEST(tokens[0].errorCode.has_value());
    BF_TEST(tokens[0].errorCode.value() == BitFlow::IO::Lexer::LexerErrorCode::MissingHexDigits);
    BF_TEST(tokens[0].span.begin == 0);
    BF_TEST(tokens[0].span.end == 2);

    return 0;
}

int TestLexer_UnexpectedCharacterHasPosition() {
    const std::vector<Token> tokens = BitFlow::IO::Lexer::Tokenize("@");

    BF_TEST(tokens.size() == 1);
    BF_TEST(tokens[0].kind == TokenKind::Error);
    BF_TEST(tokens[0].text == "Unexpected character at position 0");
    BF_TEST(tokens[0].errorCode.has_value());
    BF_TEST(tokens[0].errorCode.value() == BitFlow::IO::Lexer::LexerErrorCode::UnexpectedCharacter);
    BF_TEST(tokens[0].span.begin == 0);
    BF_TEST(tokens[0].span.end == 1);

    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestLexer_AllTokenTypes);
    BF_RUN_TEST(TestLexer_WhitespaceIsSkipped);
    BF_RUN_TEST(TestLexer_IdentifierRules);
    BF_RUN_TEST(TestLexer_HexVariants);
    BF_RUN_TEST(TestLexer_ShiftOperatorsLongestMatch);
    BF_RUN_TEST(TestLexer_InvalidInputProducesErrorToken);
    BF_RUN_TEST(TestLexer_UnexpectedCharacterHasPosition);
    return 0;
}
