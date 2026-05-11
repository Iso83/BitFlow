#pragma once

namespace BitFlow::Core::Expression {

struct PrintOptions {
    bool rotAsFunction = true;

    // --- structure ---
    bool explicitGroups = false;
    bool debugStructure = false;

    // --- debug info ---
    bool showExprIds = false;
    bool showBitWidth = false;
    bool showOpTypes = false;

    // --- fluent helpers ---
    PrintOptions& RotAsFunction(bool v = true) {
        rotAsFunction = v;
        return *this;
    }

    PrintOptions& ExplicitGroups(bool v = true) {
        explicitGroups = v;
        return *this;
    }

    PrintOptions& DebugStructure(bool v = true) {
        debugStructure = v;
        return *this;
    }

    PrintOptions& ShowExprIds(bool v = true) {
        showExprIds = v;
        return *this;
    }

    PrintOptions& ShowBitWidth(bool v = true) {
        showBitWidth = v;
        return *this;
    }

    PrintOptions& ShowOpTypes(bool v = true) {
        showOpTypes = v;
        return *this;
    }
};

} // namespace BitFlow::Core::Expression