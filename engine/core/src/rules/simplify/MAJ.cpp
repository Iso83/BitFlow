#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

struct Pair2 {
    Expr* a;
    Expr* b;
};

static bool TryGetMajPair(Expr* e, Pair2& out) {
    if (e->op == OpType::And && e->inputs.size() == 2) {
        out.a = e->inputs[0];
        out.b = e->inputs[1];
        return true;
    }

    // collapsed form: (x & x) -> x
    out.a = e;
    out.b = e;
    return true;
}

static bool SameUnorderedPair(const Pair2& p, Expr* x, Expr* y) {
    return (p.a == x && p.b == y) || (p.a == y && p.b == x);
}

static void PushUnique(std::vector<Expr*>& vars, Expr* e) {
    for (Expr* v : vars) {
        if (v == e)
            return;
    }

    vars.push_back(e);
}

static bool TryMatchMAJ(Expr* t0, Expr* t1, Expr* t2, Expr*& x, Expr*& y, Expr*& z) {
    Pair2 p0{}, p1{}, p2{};
    if (!TryGetMajPair(t0, p0) || !TryGetMajPair(t1, p1) || !TryGetMajPair(t2, p2))
        return false;

    std::vector<Expr*> vars;
    vars.reserve(6);

    PushUnique(vars, p0.a);
    PushUnique(vars, p0.b);
    PushUnique(vars, p1.a);
    PushUnique(vars, p1.b);
    PushUnique(vars, p2.a);
    PushUnique(vars, p2.b);

    for (Expr* vx : vars) {
        for (Expr* vy : vars) {
            for (Expr* vz : vars) {
                bool haveXY = false;
                bool haveXZ = false;
                bool haveYZ = false;

                const Pair2 pairs[3] = {p0, p1, p2};
                for (const Pair2& p : pairs) {
                    if (!haveXY && SameUnorderedPair(p, vx, vy)) {
                        haveXY = true;
                        continue;
                    }

                    if (!haveXZ && SameUnorderedPair(p, vx, vz)) {
                        haveXZ = true;
                        continue;
                    }

                    if (!haveYZ && SameUnorderedPair(p, vy, vz)) {
                        haveYZ = true;
                        continue;
                    }
                }

                if (haveXY && haveXZ && haveYZ) {
                    x = vx;
                    y = vy;
                    z = vz;
                    return true;
                }
            }
        }
    }

    return false;
}

static bool Match_MAJ(const Expr& e) {
    if (e.op != OpType::Xor || e.inputs.size() != 3)
        return false;

    Expr *x = nullptr, *y = nullptr, *z = nullptr;
    return TryMatchMAJ(e.inputs[0], e.inputs[1], e.inputs[2], x, y, z);
}

static Expr* Rewrite_MAJ(Expr& e) {
    Expr *x = nullptr, *y = nullptr, *z = nullptr;
    if (!TryMatchMAJ(e.inputs[0], e.inputs[1], e.inputs[2], x, y, z))
        return nullptr;

    // MAJ(x,x,y) = x
    if (x == y || x == z)
        return x;

    // MAJ(x,y,y) = y
    if (y == z)
        return y;

    // canonical reconstruct
    Expr* xy = Expression::MakeOpInterned(OpType::And, {x, y});
    Expr* xz = Expression::MakeOpInterned(OpType::And, {x, z});
    Expr* yz = Expression::MakeOpInterned(OpType::And, {y, z});

    return Expression::MakeOpInterned(OpType::Xor, {xy, xz, yz});
}

Rule Get_MAJ_Simplify_Rule() {
    return Rule{RuleId::Simplify_MAJ,
                &Match_MAJ,
                &Rewrite_MAJ,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Normalize_Order}};
}

} // namespace BitFlow::Core::Rules::Simplify