# Unified SHA demo entrypoint.
# Old projects (sha steps, demo v1, demo v2) remain as historical references,
# but only demo v3 is built by default.
add_subdirectory(sha_steps_demo_v3)

option(BF_BUILD_EXPR_TREE_EXPLORER "Build Proj visual expression tree explorer (ImGui)" OFF)
if(BF_BUILD_EXPR_TREE_EXPLORER)
    add_subdirectory(expr_tree_explorer)
endif()
