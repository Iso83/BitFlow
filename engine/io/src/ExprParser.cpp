#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/OpInfo.h>
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

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::IO::Lexer;

static std::runtime_error ParseErrorAt(const std::string& message, const Token& token) {
    return std::runtime_error(message + " at position " + std::to_string(token.span.begin));
}

class PrattParser {
  private:
    std::vector<Token> m_tokens;
    std::size_t m_pos = 0;
    std::unordered_map<std::string, ExprId> m_varIds;
    ExprStore* m_store;

  public:
    ExprNameMap m_idToName;

    explicit PrattParser(ExprStore* store, const std::string& input)
        : m_store(store), m_tokens(BitFlow::IO::Lexer::Tokenize(input)) {}

    ExprId ParseExpressionRoot() {
        if (m_tokens.empty())
            throw std::runtime_error("Internal parser error: empty token stream");

        const Token& first = Current();
        if (first.kind == TokenKind::Error)
            throw ParseErrorAt(first.text, first);

        ExprId expr = ParseExpression(0);

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

    bool Consume(TokenKind kind) {
        if (Current().kind != kind)
            return false;

        Advance();
        return true;
    }

    static bool IsPrefixToken(TokenKind kind) {
        return kind == TokenKind::Identifier || kind == TokenKind::DecimalLiteral || kind == TokenKind::HexLiteral ||
               kind == TokenKind::LeftParen || kind == TokenKind::Tilde || kind == TokenKind::Minus;
    }

    static OpType TokenToOp(TokenKind kind) {
        switch (kind) {
        case TokenKind::Star:
            return OpType::Mul;

        case TokenKind::Slash:
            return OpType::Div;

        case TokenKind::Percent:
            return OpType::Mod;

        case TokenKind::Plus:
            return OpType::Add;

        case TokenKind::Minus:
            return OpType::Sub;

        case TokenKind::ShiftLeft:
            return OpType::Shl;

        case TokenKind::ShiftRight:
            return OpType::Shr;

        case TokenKind::Ampersand:
            return OpType::And;

        case TokenKind::Caret:
            return OpType::Xor;

        case TokenKind::Pipe:
            return OpType::Or;

        default:
            throw std::runtime_error("TokenToOp: unsupported TokenKind");
        }
    }

    ExprId ParseExpression(std::uint8_t minBindingPower) {
        Token token = Current();

        if (token.kind == TokenKind::Error)
            throw ParseErrorAt(token.text, token);

        if (!IsPrefixToken(token.kind))
            throw ParseErrorAt("Expected expression", token);

        Advance();

        ExprId left = ParsePrefix(token);

        while (true) {
            const Token lookahead = Current();

            if (lookahead.kind == TokenKind::Error)
                throw ParseErrorAt(lookahead.text, lookahead);

            switch (lookahead.kind) {
            case TokenKind::Star:
            case TokenKind::Slash:
            case TokenKind::Percent:
            case TokenKind::Plus:
            case TokenKind::Minus:
            case TokenKind::ShiftLeft:
            case TokenKind::ShiftRight:
            case TokenKind::Ampersand:
            case TokenKind::Caret:
            case TokenKind::Pipe:
                break;

            default:
                return left;
            }

            const OpType op = TokenToOp(lookahead.kind);

            const OpInfo* info = GetOpInfo(op);

            if (!info || !info->infix)
                break;

            if (info->precedence <= minBindingPower)
                break;

            Advance();

            const std::uint8_t rbp = info->associativity == Associativity::Left
                                         ? info->precedence
                                         : static_cast<std::uint8_t>(info->precedence - 1);

            ExprId right = ParseExpression(rbp);

            left = m_store->create(op, {left, right}).id;
        }

        return left;
    }

    ExprId ParsePrefix(const Token& token) {
        switch (token.kind) {
        case TokenKind::Identifier:
            return ParseIdentifierOrCall(token);
        case TokenKind::DecimalLiteral:
        case TokenKind::HexLiteral:
            return ParseLiteral(token);
        case TokenKind::LeftParen:
            return ParseGroupedExpression(token);
        case TokenKind::Tilde: {
            const OpInfo* info = GetOpInfo(OpType::Not);

            if (!info)
                throw std::runtime_error("Missing OpInfo for Not");

            ExprId inner = ParseExpression(info->precedence);
            return m_store->create(OpType::Not, {inner}).id;
        }
        case TokenKind::Minus: {
            const OpInfo* info = GetOpInfo(OpType::Not);

            if (!info)
                throw std::runtime_error("Missing OpInfo for Not");

            ExprId inner = ParseExpression(info->precedence);
            return m_store->create(OpType::Neg, {inner}).id;
        }
        default:
            throw ParseErrorAt("Expected expression", token);
        }
    }

    ExprId ParseIdentifierOrCall(const Token& token) {
        if (Current().kind == TokenKind::LeftParen)
            return ParseFunctionCall(token);

        auto it = m_varIds.find(token.text);
        if (it == m_varIds.end()) {
            ExprId id = m_store->createVariable().id;
            m_varIds[token.text] = id;
            m_idToName[id] = token.text;
            return id;
        }

        return it->second;
    }

    ExprId ParseFunctionCall(const Token& identifier) {
        if (!Consume(TokenKind::LeftParen))
            throw ParseErrorAt("Expected '(' after function name", Current());

        std::vector<ExprId> args;
        if (Current().kind != TokenKind::RightParen) {
            do {
                args.push_back(ParseExpression(0));
            } while (Consume(TokenKind::Comma));
        }

        if (!Consume(TokenKind::RightParen))
            throw ParseErrorAt("Expected ')' after function call arguments", Current());

        if (identifier.text == "rotl") {
            if (args.size() != 2)
                throw ParseErrorAt("Function rotl expects exactly 2 arguments", identifier);
            return m_store->create(OpType::RotL, {args[0], args[1]}).id;
        }

        if (identifier.text == "rotr") {
            if (args.size() != 2)
                throw ParseErrorAt("Function rotr expects exactly 2 arguments", identifier);
            return m_store->create(OpType::RotR, {args[0], args[1]}).id;
        }

        throw ParseErrorAt("Unknown function: " + identifier.text, identifier);
    }

    ExprId ParseLiteral(const Token& token) {
        if (!token.numericValue)
            throw ParseErrorAt("Missing literal numeric value", token);

        return m_store->createConstant(*token.numericValue).id;
    }

    ExprId ParseGroupedExpression(const Token& leftParen) {
        (void)leftParen;

        ExprId expr = ParseExpression(0);
        const Token& closing = Current();

        if (closing.kind != TokenKind::RightParen)
            throw ParseErrorAt("Missing closing ')'", closing);

        Advance();
        return expr;
    }
};

ParseResult Parse(ExprStore* store, const std::string& input) {
    PrattParser parser(store, input);

    ParseResult result;
    result.root = ExprRef(store, parser.ParseExpressionRoot());
    result.names = std::move(parser.m_idToName);
    return result;
}

} // namespace BitFlow::IO
