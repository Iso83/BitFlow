# BitFlow Expression Tree Explorer (ImGui)

Visual tool under `Proj/` that shows `Expr` graphs as a dockable tree view using `ImGui::TreeNodeEx`.

## Build flag

This tool is **off by default**:

- `BF_BUILD_EXPR_TREE_EXPLORER=OFF`

Enable it explicitly:

```bash
cmake -S . -B build \
  -DBF_ENABLE_FETCH=FALSE \
  -DBF_BUILD_EXPR_TREE_EXPLORER=ON
```

## Dependencies (when enabled)

The CMake project expects:

- `imgui` (with GLFW + OpenGL backends available)
- `glfw3`
- `OpenGL`

## Features

- Docking layout host (`ImGuiConfigFlags_DockingEnable`)
- Expression parse input panel
- Tree rendering via `ImGui::TreeNodeEx`
- Select node + inspect details
- Expand all / Collapse all
- DAG shared-reference indication (`(ref)`)
- Graph stats (visited/unique/shared/depth)
