#pragma once

#include <BitFlow/io/lexer/Token.h>

#include <string>
#include <vector>

namespace BitFlow::IO::Lexer {

class Lexer {
  public:
    explicit Lexer(const std::string& input);

    Token NextToken();

  private:
    const std::string& m_input;
    std::size_t m_pos = 0;

    [[nodiscard]] bool AtEnd() const noexcept;
    [[nodiscard]] char Peek(std::size_t offset = 0) const noexcept;
    void Advance(std::size_t count) noexcept;

    void SkipWhitespace() noexcept;

    Token ReadIdentifier();
    Token ReadDecimalLiteral();
    Token ReadHexLiteral();
    Token ReadOperatorOrPunctuation();
    Token MakeErrorToken(std::size_t begin, std::size_t end, std::string text) const;
};

std::vector<Token> Tokenize(const std::string& input);

} // namespace BitFlow::IO::Lexer
