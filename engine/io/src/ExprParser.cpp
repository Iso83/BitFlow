#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/lexer/Lexer.h>
#include <BitFlow/io/lexer/Token.h>
#include <BitFlow/io/lexer/TokenKind.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BitFlow::IO {

using Expr = BitFlow::Core::AST::Expr;
using OpType = BitFlow::Core::AST::OpType;
using Token = BitFlow::IO::Lexer::Token;
using TokenKind = BitFlow::IO::Lexer::TokenKind;

namespace {

static Expr* MakeVar(uint32_t id) {
    Expr* e = new Expr{};
    e->id = BitFlow::Core::Ids::ExprId{id};
    e->op = OpType::Var;
    return e;
}

static Expr* MakeOp(OpType op, std::vector<Expr*> inputs) {
    Expr* e = new Expr{};
    e->op = op;
    e->inputs = std::move(inputs);
    return e;
}

static std::runtime_error ParseErrorAt(const std::string& message, const Token& token) {
    return std::runtime_error(message + " at position " + std::to_string(token.span.begin));
}

enum class Assoc {
    Left,
    Right,
};

struct InfixInfo {
    std::uint8_t precedence;
    Assoc assoc;
    OpType op;
};

class PrattParser {
  private:
    std::vector<Token> m_tokens;
    std::size_t m_pos = 0;
    std::unordered_map<std::string, uint32_t> m_varIds;
    uint32_t m_nextVarId = 1;

  public:
    std::unordered_map<uint32_t, std::string> m_idToName;

    explicit PrattParser(const std::string& input) : m_tokens(BitFlow::IO::Lexer::Tokenize(input)) {}

    Expr* ParseExpressionRoot() {
        if (m_tokens.empty())
            throw std::runtime_error("Internal parser error: empty token stream");

        const Token& first = Current();
        if (first.kind == TokenKind::Error)
            throw ParseErrorAt(first.text, first);

        Expr* expr = ParseExpression(0);

        const Token& tail = Current();
        if (tail.kind == TokenKind::Error)
            throw ParseErrorAt(tail.text, tail);

        if (tail.kind != TokenKind::EndOfInput)
            throw ParseErrorAt("Unexpected trailing input", tail);

        return expr;
    }

  private:
    const Token& Current() const {
        if (m_pos >= m_tokens.size())
            return m_tokens.back();

        return m_tokens[m_pos];
    }

    const Token& Advance() {
        const std::size_t idx = m_pos;
        if (m_pos < m_tokens.size())
            ++m_pos;

        return m_tokens[idx];
    }

    static bool IsPrefixToken(TokenKind kind) {
        return kind == TokenKind::Identifier || kind == TokenKind::DecimalLiteral || kind == TokenKind::HexLiteral ||
               kind == TokenKind::LeftParen || kind == TokenKind::Tilde;
    }

    static std::uint8_t PrefixBindingPower(TokenKind kind) {
        if (kind == TokenKind::Tilde)
            return 70;

        return 0;
    }

    static bool TryGetInfixInfo(TokenKind kind, InfixInfo& info) {
        switch (kind) {
        case TokenKind::Ampersand:
            info = InfixInfo{30, Assoc::Left, OpType::And};
            return true;
        case TokenKind::Caret:
            info = InfixInfo{20, Assoc::Left, OpType::Xor};
            return true;
        case TokenKind::Pipe:
            info = InfixInfo{10, Assoc::Left, OpType::Or};
            return true;
        default:
            return false;
        }
    }

    Expr* ParseExpression(std::uint8_t minBindingPower) {
        Token token = Current();
        if (token.kind == TokenKind::Error)
            throw ParseErrorAt(token.text, token);

        if (!IsPrefixToken(token.kind))
            throw ParseErrorAt("Expected expression", token);

        Advance();
        Expr* left = ParsePrefix(token);

        while (true) {
            Token lookahead = Current();
            if (lookahead.kind == TokenKind::Error)
                throw ParseErrorAt(lookahead.text, lookahead);

            InfixInfo infix{};
            if (!TryGetInfixInfo(lookahead.kind, infix))
                break;

            if (infix.precedence < minBindingPower)
                break;

            Advance();

            const std::uint8_t rhsMinBindingPower =
                infix.assoc == Assoc::Left ? static_cast<std::uint8_t>(infix.precedence + 1) : infix.precedence;

            Expr* right = ParseExpression(rhsMinBindingPower);
            left = MakeOp(infix.op, {left, right});
        }

        return left;
    }

    Expr* ParsePrefix(const Token& token) {
        switch (token.kind) {
        case TokenKind::Identifier:
            return ParseIdentifier(token);
        case TokenKind::DecimalLiteral:
        case TokenKind::HexLiteral:
            return ParseLiteral(token);
        case TokenKind::LeftParen:
            return ParseGroupedExpression(token);
        case TokenKind::Tilde: {
            const std::uint8_t unaryBindingPower = PrefixBindingPower(token.kind);
            Expr* inner = ParseExpression(unaryBindingPower);
            return MakeOp(OpType::Not, {inner});
        }
        default:
            throw ParseErrorAt("Expected expression", token);
        }
    }

    Expr* ParseIdentifier(const Token& token) {
        auto it = m_varIds.find(token.text);
        if (it == m_varIds.end()) {
            uint32_t id = m_nextVarId++;
            m_varIds[token.text] = id;
            m_idToName[id] = token.text;
            return MakeVar(id);
        }

        return MakeVar(it->second);
    }

    Expr* ParseLiteral(const Token& token) {
        if (!token.numericValue)
            throw ParseErrorAt("Missing literal numeric value", token);

        return BitFlow::Core::Expression::ConstPool::Get(static_cast<uint32_t>(*token.numericValue));
    }

    Expr* ParseGroupedExpression(const Token& leftParen) {
        (void)leftParen;

        Expr* expr = ParseExpression(0);
        const Token& closing = Current();

        if (closing.kind != TokenKind::RightParen)
            throw ParseErrorAt("Missing closing ')'", closing);

        Advance();
        return expr;
    }
};

} // namespace

ParseResult Parse(const std::string& input) {
    PrattParser parser(input);

    ParseResult result;
    result.root = parser.ParseExpressionRoot();
    result.idToName = std::move(parser.m_idToName);
    return result;
}

} // namespace BitFlow::IO
