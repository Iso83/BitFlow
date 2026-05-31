#pragma once

#include "ExprTestUtils.h"
#include "RuleTestUtils.h"
#include "TestAssert.h"

namespace BitFlow::Testing {

struct DocExample {
    Core::Rules::RuleKey rule;

    const char* input;
    const char* expected;
};

bool ValidateTrace(const Core::Rules::RuleEngine& engine, Core::Rules::RuleKey target,
                   const std::unordered_set<Core::Rules::RuleKey>& usedRules) {
    auto allowed = engine.CollectRequiredRules(target);

    bool ok = true;

    for (const auto& used : usedRules) {

        if (allowed.contains(used))
            continue;

        std::cout << "\nUnexpected rule detected:\n"
                  << "  Target : " << target.value << "\n"
                  << "  Used   : " << used.value << "\n";

        ok = false;
    }

    if (!usedRules.contains(target)) {

        std::cout << "\nTarget rule never executed:\n"
                  << "  Rule : " << target.value << "\n";

        ok = false;
    }

    return ok;
}

int ValidateDocExample(const DocExample& ex) {
    Core::Expression::ExprStore store;

    auto parsed = IO::Parse(&store, ex.input);

    Core::Rules::RuleEngine engine = Core::Rules::BuildExplore();

    std::unordered_set<Core::Rules::RuleKey> usedRules;

    engine.SetDebugCallback([&](Core::Rules::RuleEngine::DebugCallBack_Ctx& ctx) { usedRules.insert(ctx.key); });

    BF_SAFE_REWRITE(rewritten,
                    Core::Expression::ExprRef(&store, engine.Rewrite(&store, parsed.root.id, &parsed.names)));

    auto result = Core::Expression::ToString(rewritten.store, rewritten.id, parsed.names);

    BF_TEST(result == ex.expected);

    BF_TEST(ValidateTrace(engine, ex.rule, usedRules));

    return 0;
}

} // namespace BitFlow::Testing