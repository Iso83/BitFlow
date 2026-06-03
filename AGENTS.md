# BitFlow Agent Instructions
When creating, modifying, extending, or removing a rule, the following checklist is mandatory.

## Existing Code Style
- Do not remove code organization constructs (regions, grouping comments, file sections) unless explicitly requested.
- Do not modernize working code unless the task explicitly requires it.
- Existing infrastructure should be assumed intentional.

## Protected Infrastructure
### RuleTestUtils.h
- Keep macro `BF_REWRITE` exactly as implemented.
- Do not replace `__VA_ARGS__` with `__VA_OPT__`.
- Do not modernize or refactor this macro.
- The current implementation is intentional and verified.

### BF_Reg_RuleDocumentation.cpp
- Keep existing `#pragma region` blocks.
- Do not remove, merge or rename regions.
- Regions are used intentionally for navigation and maintenance.
- Treat region structure as part of the file organization.

## Rule Registration
Every new rule must:
- Be declared in the appropriate `Rules.h`.
- Be added to the correct RulePipeline bundle.
- Follow the existing rule ordering conventions.

## Rule Organization
Before creating a new source file:
- Check whether similar rules already exist.
- Rules that follow the same implementation pattern but target different `OpType`s should remain grouped in the same file.
- Avoid creating new files for isolated variations of an existing rule family.

Example:
- `ADD_ZERO`
- `SUB_ZERO`
- `MUL_ONE`

may belong together when they share the same implementation style.

## Documentation
Whenever a rule is added or changed:
- Update `docs/core/core_architecture.md`.
- Keep the rule list in the same order as `Rules.h`.
- Add or update the rule description when behavior changes.
- Ensure examples reflect the actual implementation.

## Tests
Every new rule requires dedicated CTests.

Requirements:
- Add at least one focused test for the new rule.
- Add additional tests for edge cases when applicable.
- Use existing test patterns and helpers.

## Validation
Before submitting changes:
- All CTests must pass.
- Investigate every failing test.
- Never modify unrelated tests simply to obtain a passing build.
- Never weaken assertions to hide failures.

## Cross-Rule Impact
Modifying existing rules is only allowed when:
- The new rule conflicts with existing behavior.
- Rule ordering/dependencies must be adjusted.
- A genuine bug is discovered.

When modifying existing rules:
- Document the reason.
- Verify the complete test suite still passes.

## Dependency Review
For every new rule:
- Review required dependencies.
- Add missing dependencies explicitly.
- Validate dependency ordering.
- Run dependency validation tests.

## Completion Criteria
A rule change is not complete until:
- Rule implementation exists.
- Rule registration is complete.
- RulePipeline integration is complete.
- Documentation is updated.
- CTests are added.
- All CTests pass.
- No unrelated tests were modified merely to achieve a pass.

## Existing Rule First
Before implementing a new rule:
- Verify that the behavior cannot already be achieved through a combination of existing rules.
- Prefer extending an existing rule when the transformation belongs to the same conceptual operation.
- Avoid introducing duplicate rewrite behavior.

## Documentation Validation
Whenever a rule is added, removed, renamed, reordered, or modified:
- Update `docs/core/core_architecture.md`.
- Keep the documented rule list in the same order as `Rules.h`.
- Update `BF_Reg_RuleDocumentation.cpp`.
- Keep `BuildRulesInDeclarationOrder()` synchronized with `Rules.h`.
- Keep `BuildDocExamples()` synchronized with `Rules.h`.
- Every rule must have at least one documentation example.
- Documentation examples must describe the behavior of the target rule itself.
- Documentation examples must remain stable under `RulePipeline::BuildExplore()`.

Purpose of `BF_Reg_RuleDocumentation.cpp`:
- Verifies that `Rules.h`, documentation, and examples remain synchronized.
- Verifies that every registered rule has documentation coverage.
- Detects duplicate rewrite behavior between rules.
- Detects hidden rule overlap.
- Detects unintended cross-rule interactions.
- Helps prevent multiple rules from implementing the same transformation.

Notes:
- `RulePipeline::BuildExplore()` is considered the canonical rewrite pipeline and is used by the validation tests.
- `RulePipeline::BuildExpand()` is an optional user-facing expansion pipeline.
- Expansion rules may intentionally introduce flip-flop behavior or intermediate forms that are later simplified by Explore.
- Documentation examples should therefore be validated against Explore behavior, not Expand behavior.

## Existing Rule First
Before implementing a new rule:
- Verify that the behavior cannot already be achieved through a combination of existing rules.
- Verify that no existing rule already performs the same rewrite.
- Run `BF_Reg_RuleDocumentation.cpp` and review failures carefully.
- Prefer extending an existing rule when the transformation belongs to the same conceptual operation.
- Avoid introducing duplicate rewrite behavior.
- Avoid introducing specialized rules that are already covered by a more generic rule.