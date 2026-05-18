#include <BitFlow/io/lexer/Lexer.h>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <utility>

namespace BitFlow::IO::Lexer {

bool IsIdentifierStart(char ch) {
    const unsigned char uch = static_cast<unsigned char>(ch);
    return std::isalpha(uch) || ch == '_';
}

bool IsIdentifierPart(char ch) {
    const unsigned char uch = static_cast<unsigned char>(ch);
    return std::isalnum(uch) || ch == '_';
}

bool IsDecDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool IsHexDigit(char ch) {
    const unsigned char uch = static_cast<unsigned char>(ch);
    return std::isxdigit(uch) != 0;
}

std::optional<std::uint64_t> ParseUnsigned(const std::string& text, int base) {
    std::uint64_t value = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value, base);

    if (result.ec != std::errc{} || result.ptr != end)
        return std::nullopt;

    return value;
}

std::string BuildErrorText(const std::string& message, std::size_t position) {
    return message + " at position " + std::to_string(position);
}

Lexer::Lexer(const std::string& input) : m_input(input) {}

Token Lexer::NextToken() {
    SkipWhitespace();

    if (AtEnd())
        return Token{TokenKind::EndOfInput, {}, SourceSpan{m_pos, m_pos}, std::nullopt, std::nullopt};

    if (IsIdentifierStart(Peek()))
        return ReadIdentifier();

    if (IsDecDigit(Peek())) {
        if (Peek() == '0' && (Peek(1) == 'x' || Peek(1) == 'X'))
            return ReadHexLiteral();

        return ReadDecimalLiteral();
    }

    return ReadOperatorOrPunctuation();
}

bool Lexer::AtEnd() const noexcept {
    return m_pos >= m_input.size();
}

char Lexer::Peek(std::size_t offset) const noexcept {
    const std::size_t idx = m_pos + offset;
    return idx < m_input.size() ? m_input[idx] : '\0';
}

void Lexer::Advance(std::size_t count) noexcept {
    m_pos += count;
}

void Lexer::SkipWhitespace() noexcept {
    while (!AtEnd() && std::isspace(static_cast<unsigned char>(Peek())))
        Advance(1);
}

Token Lexer::ReadIdentifier() {
    const std::size_t begin = m_pos;

    while (IsIdentifierPart(Peek()))
        Advance(1);

    std::string text = m_input.substr(begin, m_pos - begin);
    return Token{TokenKind::Identifier, std::move(text), SourceSpan{begin, m_pos}, std::nullopt, std::nullopt};
}

Token Lexer::ReadDecimalLiteral() {
    const std::size_t begin = m_pos;

    while (IsDecDigit(Peek()))
        Advance(1);

    std::string text = m_input.substr(begin, m_pos - begin);
    const std::optional<std::uint64_t> value = ParseUnsigned(text, 10);

    if (!value)
        return MakeErrorToken(begin, m_pos, LexerErrorCode::InvalidDecimalLiteral,
                              BuildErrorText("Invalid decimal literal", begin));

    return Token{TokenKind::DecimalLiteral, std::move(text), SourceSpan{begin, m_pos}, value, std::nullopt};
}

Token Lexer::ReadHexLiteral() {
    const std::size_t begin = m_pos;
    Advance(2); // 0x / 0X

    const std::size_t hexBegin = m_pos;
    while (IsHexDigit(Peek()))
        Advance(1);

    if (hexBegin == m_pos)
        return MakeErrorToken(begin, m_pos, LexerErrorCode::MissingHexDigits,
                              BuildErrorText("Expected hex digits after 0x prefix", begin));

    std::string text = m_input.substr(begin, m_pos - begin);
    const std::string hexDigits = text.substr(2);
    const std::optional<std::uint64_t> value = ParseUnsigned(hexDigits, 16);

    if (!value)
        return MakeErrorToken(begin, m_pos, LexerErrorCode::InvalidHexLiteral,
                              BuildErrorText("Invalid hex literal", begin));

    return Token{TokenKind::HexLiteral, std::move(text), SourceSpan{begin, m_pos}, value, std::nullopt};
}

Token Lexer::ReadOperatorOrPunctuation() {
    const std::size_t begin = m_pos;

    const auto makeSimple = [&](TokenKind kind, std::size_t length) -> Token {
        std::string text = m_input.substr(begin, length);
        Advance(length);
        return Token{kind, std::move(text), SourceSpan{begin, begin + length}, std::nullopt, std::nullopt};
    };

    if (Peek() == '<' && Peek(1) == '<' && Peek(2) == '<')
        return makeSimple(TokenKind::RotLeft, 3);

    if (Peek() == '>' && Peek(1) == '>' && Peek(2) == '>')
        return makeSimple(TokenKind::RotRight, 3);

    if (Peek() == '*' && Peek(1) == '*')
        return makeSimple(TokenKind::Pow, 2);

    if (Peek() == '>' && Peek(1) == '>')
        return makeSimple(TokenKind::ShiftRight, 2);

    if (Peek() == '<' && Peek(1) == '<')
        return makeSimple(TokenKind::ShiftLeft, 2);

    switch (Peek()) {
    case '(':
        return makeSimple(TokenKind::LeftParen, 1);
    case ')':
        return makeSimple(TokenKind::RightParen, 1);
    case ',':
        return makeSimple(TokenKind::Comma, 1);
    case '+':
        return makeSimple(TokenKind::Plus, 1);
    case '-':
        return makeSimple(TokenKind::Minus, 1);
    case '*':
        return makeSimple(TokenKind::Star, 1);
    case '/':
        return makeSimple(TokenKind::Slash, 1);
    case '%':
        return makeSimple(TokenKind::Percent, 1);
    case '&':
        return makeSimple(TokenKind::Ampersand, 1);
    case '|':
        return makeSimple(TokenKind::Pipe, 1);
    case '^':
        return makeSimple(TokenKind::Caret, 1);
    case '~':
        return makeSimple(TokenKind::Tilde, 1);
    default:
        Advance(1);
        return MakeErrorToken(begin, m_pos, LexerErrorCode::UnexpectedCharacter,
                              BuildErrorText("Unexpected character", begin));
    }
}

Token Lexer::MakeErrorToken(std::size_t begin, std::size_t end, LexerErrorCode errorCode, std::string text) const {
    return Token{TokenKind::Error, std::move(text), SourceSpan{begin, end}, std::nullopt, errorCode};
}

std::vector<Token> Tokenize(const std::string& input) {
    Lexer lexer(input);
    std::vector<Token> tokens;

    while (true) {
        Token token = lexer.NextToken();
        const TokenKind kind = token.kind;
        tokens.push_back(std::move(token));

        if (kind == TokenKind::EndOfInput || kind == TokenKind::Error)
            break;
    }

    return tokens;
}

} // namespace BitFlow::IO::Lexer
