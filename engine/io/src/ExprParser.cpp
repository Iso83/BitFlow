#include <BitFlow/engine/core/expression/OpInfo.h>
#include <BitFlow/engine/io/ExprParser.h>
#include <BitFlow/engine/io/helper/Exception.h>
#include <BitFlow/engine/io/lexer/Lexer.h>
#include <algorithm>
#include <optional>
#include <unordered_map>

namespace BitFlow::Engine::IO {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::IO::Lexer;
namespace Types = BitFlow::Engine::Core::Types;

static IOException ParseErrorAt(const std::string& message, const Token& token) {
    return IOException(message + " at position " + std::to_string(token.span.begin));
}

class PrattParser {
private:
    std::vector<Token> m_tokens;
    std::size_t m_pos = 0;
    std::unordered_map<std::string, std::unordered_map<Types::BitWidth, ExprId>> m_varIds;
    ExprStore* m_store;
    IFunctionResolver* m_functions{};

public:
    ExprNameMap m_idToName;

    explicit PrattParser(ExprStore* store, const std::string& input, IFunctionResolver* functions = nullptr)
        : m_store(store), m_functions(functions), m_tokens(BitFlow::Engine::IO::Lexer::Tokenize(input)) {}

    ExprId ParseExpressionRoot() {
        if (m_tokens.empty())
            BF_IO_THROW("Internal parser error: empty token stream");

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

    const Token& PeekToken(std::size_t offset = 0) const {
        const std::size_t idx = m_pos + offset;
        if (idx >= m_tokens.size())
            return m_tokens.back();

        return m_tokens[idx];
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
               kind == TokenKind::BinaryLiteral || kind == TokenKind::LeftParen || kind == TokenKind::Tilde ||
               kind == TokenKind::Plus || kind == TokenKind::Minus;
    }

    static bool IsInfixOperandStart(const Token& token) {
        return IsPrefixToken(token.kind);
    }

    bool IsMulAliasXToken(const Token& token) const {
        if (token.kind != TokenKind::Identifier || token.text != "x")
            return false;

        return IsInfixOperandStart(PeekToken(1));
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

            case TokenKind::RotLeft:
                return OpType::RotL;

            case TokenKind::RotRight:
                return OpType::RotR;

            case TokenKind::Ampersand:
                return OpType::And;

            case TokenKind::Caret:
                return OpType::Xor;

            case TokenKind::Pipe:
                return OpType::Or;

            case TokenKind::Pow:
                return OpType::Pow;

            default:
                BF_IO_THROW("TokenToOp: unsupported TokenKind");
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

            bool isMulAliasX = false;

            switch (lookahead.kind) {
                case TokenKind::Star:
                case TokenKind::Slash:
                case TokenKind::Percent:
                case TokenKind::Plus:
                case TokenKind::Minus:
                case TokenKind::ShiftLeft:
                case TokenKind::ShiftRight:
                case TokenKind::RotLeft:
                case TokenKind::RotRight:
                case TokenKind::Ampersand:
                case TokenKind::Caret:
                case TokenKind::Pipe:
                case TokenKind::Pow:
                    break;

                default:
                    if (!IsMulAliasXToken(lookahead))
                        return left;

                    isMulAliasX = true;
                    break;
            }

            const OpType op = isMulAliasX ? OpType::Mul : TokenToOp(lookahead.kind);

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

            left = CreateBinary(op, left, right);
        }

        return left;
    }

    ExprId ParseSignedPrefixExpression(bool isNegative) {
        while (Current().kind == TokenKind::Plus || Current().kind == TokenKind::Minus) {
            if (Current().kind == TokenKind::Minus)
                isNegative = !isNegative;

            Advance();
        }

        const OpInfo* info = GetOpInfo(OpType::Neg);

        if (!info)
            BF_IO_THROW("Missing OpInfo for Neg");

        ExprId inner = ParseExpression(info->precedence);

        if (!isNegative)
            return inner;

        return CreateUnary(OpType::Neg, inner);
    }

    ExprId ParsePrefix(const Token& token) {
        switch (token.kind) {
            case TokenKind::Identifier:
                return ParseIdentifierOrCall(token);
            case TokenKind::DecimalLiteral:
            case TokenKind::HexLiteral:
            case TokenKind::BinaryLiteral:
                return ParseLiteral(token);
            case TokenKind::LeftParen:
                return ParseGroupedExpression(token);
            case TokenKind::Tilde:
                {
                    const OpInfo* info = GetOpInfo(OpType::Not);

                    if (!info)
                        BF_IO_THROW("Missing OpInfo for Not");

                    ExprId inner = ParseExpression(info->precedence);
                    return CreateUnary(OpType::Not, inner);
                }
            case TokenKind::Plus:
                return ParseSignedPrefixExpression(false);
            case TokenKind::Minus:
                return ParseSignedPrefixExpression(true);
            default:
                throw ParseErrorAt("Expected expression", token);
        }
    }

    ExprId CreateUnary(OpType op, ExprId input) {
        const Expr& inputExpr = (*m_store)[input];
        return m_store->create(op, {input}, inputExpr.bitWidth).id;
    }

    ExprId CreateBinary(OpType op, ExprId left, ExprId right) {
        const Expr& leftExpr = (*m_store)[left];
        const Expr& rightExpr = (*m_store)[right];

        Types::BitWidth bitWidth = Types::ExprChunkBits;

        switch (op) {
            case OpType::Shl:
            case OpType::Shr:
            case OpType::RotL:
            case OpType::RotR:
            case OpType::Pow:
                bitWidth = leftExpr.bitWidth;
                break;
            default:
                bitWidth = std::max(leftExpr.bitWidth, rightExpr.bitWidth);
                break;
        }

        return m_store->create(op, {left, right}, bitWidth).id;
    }

    static std::optional<Types::BitWidth> WidthConstructorBitWidth(const std::string& name) {
        if (name == "u8")
            return Types::BitWidth{8};

        if (name == "u16")
            return Types::BitWidth{16};

        if (name == "u32")
            return Types::BitWidth{32};

        if (name == "u64")
            return Types::BitWidth{64};

        return std::nullopt;
    }

    ExprId GetOrCreateVariable(const std::string& name, Types::BitWidth bitWidth) {
        auto& widths = m_varIds[name];
        auto it = widths.find(bitWidth);
        if (it != widths.end())
            return it->second;

        ExprId id = m_store->createVariable(bitWidth).id;
        widths[bitWidth] = id;
        m_idToName[id] = name;
        return id;
    }

    ExprId ParseWidthConstructor(const Token& identifier, Types::BitWidth bitWidth) {
        if (!Consume(TokenKind::LeftParen))
            throw ParseErrorAt("Expected '(' after width constructor", Current());

        if (Current().kind == TokenKind::RightParen)
            throw ParseErrorAt("Width constructor expects exactly 1 argument", identifier);

        ExprId inner = ParseExpression(0);

        if (Consume(TokenKind::Comma))
            throw ParseErrorAt("Width constructor expects exactly 1 argument", Current());

        if (!Consume(TokenKind::RightParen))
            throw ParseErrorAt("Expected ')' after width constructor argument", Current());

        return ApplyExplicitWidth(inner, bitWidth);
    }

    ExprId ApplyExplicitWidth(ExprId id, Types::BitWidth bitWidth) {
        const Expr& expr = (*m_store)[id];

        if (expr.op == OpType::Const) {
            const Types::ExprChunk mask = Expr::fullMask(bitWidth);
            return m_store->createConstant(expr.knownValue & mask, bitWidth).id;
        }

        if (expr.op == OpType::Var) {
            const auto name = m_idToName.find(id);
            if (name != m_idToName.end())
                return GetOrCreateVariable(name->second, bitWidth);
        }

        ExprInputs inputs = expr.inputs;
        return m_store->create(expr.op, std::move(inputs), bitWidth).id;
    }

    ExprId ParseIdentifierOrCall(const Token& token) {
        if (Current().kind == TokenKind::LeftParen) {
            if (auto width = WidthConstructorBitWidth(token.text))
                return ParseWidthConstructor(token, *width);

            return ParseFunctionCall(token);
        }

        return GetOrCreateVariable(token.text, Types::ExprChunkBits);
    }

    ExprId ParseFunctionCall(const Token& identifier) {
        if (!Consume(TokenKind::LeftParen))
            throw ParseErrorAt("Expected '(' after function name", Current());

        std::vector<ExprRef> args;

        if (Current().kind != TokenKind::RightParen) {
            do {
                ExprId id = ParseExpression(0);
                args.emplace_back(m_store, id);
            } while (Consume(TokenKind::Comma));
        }

        if (!Consume(TokenKind::RightParen))
            throw ParseErrorAt("Expected ')' after function call arguments", Current());

        if (identifier.text == "pow") {
            if (args.size() != 2)
                throw ParseErrorAt("Function pow expects exactly 2 arguments", identifier);

            return CreateBinary(OpType::Pow, args[0].id, args[1].id);
        }

        if (!m_functions || !m_functions->Contains(identifier.text))
            throw ParseErrorAt("Unknown function: " + identifier.text, identifier);

        ExprRef result = m_functions->Resolve({.store = m_store, .name = identifier.text, .args = args});

        if (!result.IsValid() || result.store != m_store)
            throw ParseErrorAt("Function returned invalid expression: " + identifier.text, identifier);

        return result.id;
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

ParseResult Parse(ExprStore* store, const std::string& input, IFunctionResolver* functions) {
    PrattParser parser(store, input, functions);

    ParseResult result;
    result.root = ExprRef(store, parser.ParseExpressionRoot());
    result.names = std::move(parser.m_idToName);
    return result;
}

} // namespace BitFlow::Engine::IO
