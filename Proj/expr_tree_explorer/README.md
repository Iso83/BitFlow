# BitFlow Expression Tree Explorer (ImGui)

Visual tool under `Proj/` that shows `Expr` graphs as a dockable tree view using `ImGui::TreeNodeEx`.

## Build flag

This tool is **off by default**:

- `BF_BUILD_EXPR_TREE_EXPLORER=OFF`

Enable it explicitly:

```bash
cmake -S . -B build \
  -DBF_ENABLE_FETCH=TRUE \
  -DBF_BUILD_EXPR_TREE_EXPLORER=ON
```

## Dependencies (when enabled)

The explorer CMake fetches these dependencies automatically via `FetchContent`:

- GLFW `3.3.9`
- ImGui docking snapshot `60d7fb207eeb46d6363dd4bde10b35991bae0ce7`

System dependency still required:

- OpenGL development/runtime

## Features

- Docking layout host (`ImGuiConfigFlags_DockingEnable`)
- Expression parse input panel
- Tree rendering via `ImGui::TreeNodeEx`
- Select node + inspect details
- Expand all / Collapse all
- DAG shared-reference indication (`(ref)`)
- Graph stats (visited/unique/shared/depth)
