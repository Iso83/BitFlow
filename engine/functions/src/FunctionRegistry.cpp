#include <BitFlow/engine/core/helper/Debug.h>
#include <BitFlow/engine/functions/FunctionRegistry.h>

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

namespace BitFlow::Engine::Functions {

bool FunctionRegistry::Add(FunctionDefinition def) {
    BF_CORE_ASSERT(!def.name.empty());
    BF_CORE_ASSERT(def.expand != nullptr);

    if (Contains(def.name))
        return false;

    m_lookup.emplace(std::string(def.name), m_functions.size());

    m_functions.push_back(def);

    return true;
}

void FunctionRegistry::Merge(const FunctionRegistry& other) {
    for (const auto& fn : other.m_functions)
        Add(fn);
}

bool FunctionRegistry::Contains(std::string_view name) const {
    return m_lookup.find(std::string(name)) != m_lookup.end();
}

const FunctionDefinition* FunctionRegistry::Find(std::string_view name) const {
    auto it = m_lookup.find(std::string(name));

    if (it == m_lookup.end())
        return nullptr;

    return &m_functions[it->second];
}

FunctionRef FunctionRegistry::Get(std::string_view name) {
    const auto* fn = Find(name);

    BF_CORE_ASSERT(fn != nullptr);

    return FunctionRef(this, fn);
}

ExprRef FunctionRegistry::Resolve(FunctionResolveContext ctx) {
    const auto* fn = Find(ctx.name);

    BF_CORE_ASSERT(fn != nullptr);

    return Invoke(*fn, ctx.store, ctx.args);
}

ExprRef FunctionRegistry::Invoke(const FunctionDefinition& def, ExprStore* store, std::span<const ExprRef> args) {
    BF_CORE_ASSERT(def.expand != nullptr);
    BF_CORE_ASSERT(args.size() == def.parameterCount);

    if (store == nullptr && !args.empty())
        store = args.front().store;

    BF_CORE_ASSERT(store != nullptr);

    ExprInputs ids;
    ids.reserve(args.size());

    for (const auto& arg : args) {
        BF_CORE_ASSERT(arg.store == store);
        ids.push_back(arg.id);
    }

    FunctionExpandContext ctx{.store = store, .args = ids};

    ExprRef result = def.expand(ctx);

    BF_CORE_ASSERT(result.IsValid());
    BF_CORE_ASSERT(result.store == store);

    FunctionCallInfo call;
    call.name = std::string(def.name);
    call.result = result;
    call.parameters.assign(args.begin(), args.end());

    m_calls.push_back(std::move(call));

    return result;
}

} // namespace BitFlow::Engine::Functions
