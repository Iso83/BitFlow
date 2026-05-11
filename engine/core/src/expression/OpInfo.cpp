#include <BitFlow/core/expression/OpInfo.h>

namespace BitFlow::Core::Expression {

static constexpr OpInfo g_not{90, Associativity::Right, "~", false, true};

static constexpr OpInfo g_neg{90, Associativity::Right, "-", false, true};

static constexpr OpInfo g_mul{80, Associativity::Left, "*", true, false};

static constexpr OpInfo g_div{80, Associativity::Left, "/", true, false};

static constexpr OpInfo g_mod{80, Associativity::Left, "%", true, false};

static constexpr OpInfo g_add{70, Associativity::Left, "+", true, false};

static constexpr OpInfo g_sub{70, Associativity::Left, "-", true, false};

static constexpr OpInfo g_shl{60, Associativity::Left, "<<", true, false};

static constexpr OpInfo g_shr{60, Associativity::Left, ">>", true, false};

static constexpr OpInfo g_and{50, Associativity::Left, "&", true, false};

static constexpr OpInfo g_xor{40, Associativity::Left, "^", true, false};

static constexpr OpInfo g_or{30, Associativity::Left, "|", true, false};

const OpInfo* GetOpInfo(OpType op) {
    switch (op) {
    case OpType::Not:
        return &g_not;

    case OpType::Neg:
        return &g_neg;

    case OpType::Mul:
        return &g_mul;

    case OpType::Div:
        return &g_div;

    case OpType::Mod:
        return &g_mod;

    case OpType::Add:
        return &g_add;

    case OpType::Sub:
        return &g_sub;

    case OpType::Shl:
        return &g_shl;

    case OpType::Shr:
        return &g_shr;

    case OpType::And:
        return &g_and;

    case OpType::Xor:
        return &g_xor;

    case OpType::Or:
        return &g_or;

    default:
        return nullptr;
    }
}

bool RequiresParentheses(OpType parent, OpType child, bool isRightChild) {

    const OpInfo* p = GetOpInfo(parent);
    const OpInfo* c = GetOpInfo(child);

    if (!p || !c)
        return false;

    if (c->precedence < p->precedence)
        return true;

    if (c->precedence > p->precedence)
        return false;

    if (!isRightChild)
        return false;

    if (parent == child) {
        return !IsAssociative(parent);
    }

    return true;
}

} // namespace BitFlow::Core::Expression