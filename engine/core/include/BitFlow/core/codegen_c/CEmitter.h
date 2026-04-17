#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::CodegenC {

struct CParameter {
    uint32_t id = 0;
    std::string type;
    std::string name;
};

struct CLocal {
    std::string type;
    std::string name;
    std::string expr;
};

struct CFunction {
    std::string returnType;
    std::string functionName;
    std::vector<CParameter> parameters;
    std::vector<CLocal> locals;
    std::string returnExpr;
};

std::vector<CParameter> BuildCParameters(const AST::Expr* root,
                                         const std::map<uint32_t, std::string>& names = {});
std::vector<CLocal> BuildCLocals(const AST::Expr* root, uint32_t bitWidth);
CFunction BuildCFunction(const AST::Expr* root, uint32_t bitWidth, const std::string& functionName = "f",
                         const std::map<uint32_t, std::string>& names = {});
std::string EmitCFunction(const CFunction& function);

} // namespace BitFlow::Core::CodegenC
