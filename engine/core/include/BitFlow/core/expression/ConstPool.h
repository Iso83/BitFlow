#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <unordered_map>

namespace BitFlow::Core::Expression {

class ConstPool {
  public:
    static AST::Expr* Get(uint32_t value) {
        auto it = pool().find(value);
        if (it != pool().end())
            return it->second;

        AST::Expr* e = new AST::Expr{};
        e->op = AST::OpType::Const;
        e->constValue = value;
        e->id = Ids::ExprId{NextId()};

        pool()[value] = e;
        return e;
    }

  private:
    static std::unordered_map<uint32_t, AST::Expr*>& pool() {
        static std::unordered_map<uint32_t, AST::Expr*> p;
        return p;
    }

    static uint32_t NextId() {
        static uint32_t id = 1000000;
        return id++;
    }
};

} // namespace BitFlow::Core::Expression
