#pragma once

#include <BitFlow/core/Expression.h>
#include <unordered_map>

namespace BitFlow::Core {

class ConstPool {
  public:
    static Expr* Get(uint32_t value) {
        auto it = pool().find(value);
        if (it != pool().end())
            return it->second;

        Expr* e = new Expr{};
        e->isConst = true;
        e->constValue = value;
        e->id = Ids::ExprId{NextId()};

        pool()[value] = e;
        return e;
    }

  private:
    static std::unordered_map<uint32_t, Expr*>& pool() {
        static std::unordered_map<uint32_t, Expr*> p;
        return p;
    }

    static uint32_t NextId() {
        static uint32_t id = 1000000; // aparte range voor constants
        return id++;
    }
};

} // namespace BitFlow::Core