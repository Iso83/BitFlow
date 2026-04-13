#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/io/ExprParser.h>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace BitFlow::IO {

using Expr = BitFlow::Core::AST::Expr;
using OpType = BitFlow::Core::AST::OpType;

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

class Parser {
  private:
    const std::string& m_input;
    size_t m_pos = 0;
    std::unordered_map<std::string, uint32_t> m_varIds;
    uint32_t m_nextVarId = 1;

  public:
    std::unordered_map<uint32_t, std::string> m_idToName;

    explicit Parser(const std::string& input) : m_input(input) {}

    Expr* ParseExpr() {
        Expr* e = ParseOr();
        SkipWs();

        if (!AtEnd())
            throw std::runtime_error("Unexpected trailing input");

        return e;
    }

  private:
    bool AtEnd() const {
        return m_pos >= m_input.size();
    }

    char Peek() const {
        return AtEnd() ? '\0' : m_input[m_pos];
    }

    char Get() {
        return AtEnd() ? '\0' : m_input[m_pos++];
    }

    void SkipWs() {
        while (!AtEnd() && std::isspace(static_cast<unsigned char>(m_input[m_pos])))
            ++m_pos;
    }

    bool Consume(char ch) {
        SkipWs();
        if (Peek() != ch)
            return false;

        ++m_pos;
        return true;
    }

    Expr* ParseOr() {
        Expr* left = ParseXor();

        while (true) {
            SkipWs();
            if (!Consume('|'))
                break;

            Expr* right = ParseXor();
            left = MakeOp(OpType::Or, {left, right});
        }

        return left;
    }

    Expr* ParseXor() {
        Expr* left = ParseAnd();

        while (true) {
            SkipWs();
            if (!Consume('^'))
                break;

            Expr* right = ParseAnd();
            left = MakeOp(OpType::Xor, {left, right});
        }

        return left;
    }

    Expr* ParseAnd() {
        Expr* left = ParseUnary();

        while (true) {
            SkipWs();
            if (!Consume('&'))
                break;

            Expr* right = ParseUnary();
            left = MakeOp(OpType::And, {left, right});
        }

        return left;
    }

    Expr* ParseUnary() {
        SkipWs();

        if (Consume('~')) {
            Expr* inner = ParseUnary();
            return MakeOp(OpType::Not, {inner});
        }

        return ParsePrimary();
    }

    Expr* ParsePrimary() {
        SkipWs();

        if (Peek() == '+' || Peek() == '*' || Peek() == '-' || Peek() == '/')
            throw std::runtime_error("Unsupported operator");

        if (Consume('(')) {
            Expr* e = ParseOr();
            SkipWs();

            if (!Consume(')'))
                throw std::runtime_error("Missing closing ')'");

            return e;
        }

        if (std::isdigit(static_cast<unsigned char>(Peek())))
            return ParseNumber();

        if (std::isalpha(static_cast<unsigned char>(Peek())) || Peek() == '_')
            return ParseIdentifier();

        throw std::runtime_error("Unexpected token");
    }

    Expr* ParseNumber() {
        SkipWs();

        uint32_t value = 0;
        bool hasDigit = false;

        while (std::isdigit(static_cast<unsigned char>(Peek()))) {
            hasDigit = true;
            value = value * 10u + static_cast<uint32_t>(Get() - '0');
        }

        if (!hasDigit)
            throw std::runtime_error("Expected number");

        return BitFlow::Core::Expression::ConstPool::Get(value);
    }

    Expr* ParseIdentifier() {
        SkipWs();

        std::string name;
        while (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_')
            name.push_back(Get());

        if (name.empty())
            throw std::runtime_error("Expected identifier");

        auto it = m_varIds.find(name);
        if (it == m_varIds.end()) {
            uint32_t id = m_nextVarId++;
            m_varIds[name] = id;
            m_idToName[id] = name;
            return MakeVar(id);
        }

        return MakeVar(it->second);
    }
};

} // namespace

ParseResult Parse(const std::string& input) {
    Parser p(input);

    ParseResult r;
    r.root = p.ParseExpr();
    r.idToName = std::move(p.m_idToName);
    return r;
}

} // namespace BitFlow::IO