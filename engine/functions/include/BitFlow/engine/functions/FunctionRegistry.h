#pragma once

#include <BitFlow/engine/functions/FunctionDefinition.h>
#include <BitFlow/engine/io/IFunctionResolver.h>
#include <array>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace BitFlow::Engine::Functions {

using IFunctionResolver = BitFlow::Engine::IO::IFunctionResolver;
using FunctionResolveContext = BitFlow::Engine::IO::FunctionResolveContext;

struct FunctionCallInfo {
    std::string name;
    Core::Expression::ExprRef result;
    std::vector<Core::Expression::ExprRef> parameters;
};

class FunctionRef;

class FunctionRegistry : public IFunctionResolver {
  private:
    friend class FunctionRef;

    [[nodiscard]]
    Core::Expression::ExprRef Invoke(const FunctionDefinition& def, Core::Expression::ExprStore* store,
                                     std::span<const Core::Expression::ExprRef> args);

  private:
    std::vector<FunctionDefinition> m_functions;
    std::unordered_map<std::string, size_t> m_lookup;

    std::vector<FunctionCallInfo> m_calls;

  public:
    bool Add(FunctionDefinition def);

    void Merge(const FunctionRegistry& other);

    [[nodiscard]]
    bool Contains(std::string_view name) const override;

    [[nodiscard]]
    const FunctionDefinition* Find(std::string_view name) const;

    [[nodiscard]]
    FunctionRef Get(std::string_view name);

    [[nodiscard]]
    Core::Expression::ExprRef Resolve(FunctionResolveContext ctx) override;

    [[nodiscard]]
    const std::vector<FunctionCallInfo>& Calls() const noexcept {
        return m_calls;
    }

    void ClearCalls() {
        m_calls.clear();
    }
};

class FunctionRef {
  private:
    FunctionRegistry* m_registry{};
    const FunctionDefinition* m_def{};

  public:
    FunctionRef() = default;

    FunctionRef(FunctionRegistry* registry, const FunctionDefinition* def) : m_registry(registry), m_def(def) {}

    [[nodiscard]]
    bool IsValid() const noexcept {
        return m_registry != nullptr && m_def != nullptr;
    }

    template <typename... TArgs>
    [[nodiscard]]
    Core::Expression::ExprRef operator()(TArgs&&... args) const {
        std::array<Core::Expression::ExprRef, sizeof...(args)> refs{std::forward<TArgs>(args)...};

        return m_registry->Invoke(*m_def, nullptr,
                                  std::span<const Core::Expression::ExprRef>(refs.data(), refs.size()));
    }
};

} // namespace BitFlow::Functions
