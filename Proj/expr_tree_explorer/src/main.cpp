#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstdint>
#include <exception>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

using BitFlow::Core::AST::Expr;
using BitFlow::Core::AST::OpType;

struct GraphStats {
    int totalNodes = 0;
    int uniqueNodes = 0;
    int sharedRefs = 0;
    int maxDepth = 0;
};

struct ExplorerState {
    std::array<char, 8192> input{};
    std::string error;
    std::unordered_map<uint32_t, std::string> names;
    Expr* root = nullptr;

    const Expr* selected = nullptr;
    int selectedDepth = 0;

    bool requestExpandAll = false;
    bool requestCollapseAll = false;

    float uiScale = 1.0f;
    bool dockLayoutBuilt = false;
    GraphStats stats{};
};

const char* OpName(OpType op) {
    switch (op) {
    case OpType::Var:
        return "VAR";
    case OpType::Const:
        return "CONST";
    case OpType::Not:
        return "NOT";
    case OpType::Neg:
        return "NEG";
    case OpType::And:
        return "AND";
    case OpType::Or:
        return "OR";
    case OpType::Xor:
        return "XOR";
    case OpType::Add:
        return "ADD";
    case OpType::Sub:
        return "SUB";
    case OpType::Mul:
        return "MUL";
    case OpType::Div:
        return "DIV";
    case OpType::Mod:
        return "MOD";
    case OpType::Shl:
        return "SHL";
    case OpType::Shr:
        return "SHR";
    case OpType::UShr:
        return "USHR";
    case OpType::RotL:
        return "ROTL";
    case OpType::RotR:
        return "ROTR";
    case OpType::Ch:
        return "CH";
    case OpType::Maj:
        return "MAJ";
    }
    return "UNKNOWN";
}

ImVec4 OpColor(OpType op) {
    switch (op) {
    case OpType::Var:
        return ImVec4(0.40f, 0.86f, 0.42f, 1.0f);
    case OpType::Const:
        return ImVec4(0.98f, 0.72f, 0.18f, 1.0f);
    case OpType::Add:
    case OpType::Sub:
    case OpType::Mul:
    case OpType::Div:
    case OpType::Mod:
        return ImVec4(0.30f, 0.72f, 1.0f, 1.0f);
    case OpType::Xor:
    case OpType::And:
    case OpType::Or:
        return ImVec4(0.24f, 0.66f, 0.96f, 1.0f);
    case OpType::Ch:
    case OpType::Maj:
        return ImVec4(0.88f, 0.36f, 0.78f, 1.0f);
    default:
        return ImVec4(0.82f, 0.84f, 0.88f, 1.0f);
    }
}

const char* OpIcon(OpType op) {
    switch (op) {
    case OpType::Var:
        return "●";
    case OpType::Const:
        return "■";
    case OpType::Add:
    case OpType::Sub:
    case OpType::Mul:
    case OpType::Div:
    case OpType::Mod:
        return "⊕";
    case OpType::Xor:
    case OpType::And:
    case OpType::Or:
        return "◇";
    case OpType::RotR:
    case OpType::RotL:
        return "↻";
    case OpType::Ch:
    case OpType::Maj:
        return "◆";
    default:
        return "•";
    }
}

std::string NodeLabel(const Expr* node, const std::unordered_map<uint32_t, std::string>& names) {
    if (node == nullptr)
        return "<null>";

    if (node->op == OpType::Var) {
        const auto it = names.find(node->id.value());
        if (it != names.end())
            return it->second;
        return "var_" + std::to_string(node->id.value());
    }
    if (node->op == OpType::Const)
        return std::to_string(node->constValue);

    return OpName(node->op);
}

void CollectStats(const Expr* node, std::unordered_set<uint32_t>& seen, GraphStats& stats, int depth) {
    if (node == nullptr)
        return;

    stats.totalNodes++;
    stats.maxDepth = std::max(stats.maxDepth, depth);

    const bool inserted = seen.insert(node->id.value()).second;
    if (inserted)
        stats.uniqueNodes++;
    else
        stats.sharedRefs++;

    for (const Expr* child : node->inputs)
        CollectStats(child, seen, stats, depth + 1);
}

void ParseExpression(ExplorerState& state) {
    try {
        auto parsed = BitFlow::IO::Parse(state.input.data());
        state.root = parsed.root;
        state.names = std::move(parsed.idToName);
        state.error.clear();
        state.selected = state.root;
        state.selectedDepth = 0;

        state.stats = {};
        std::unordered_set<uint32_t> seen;
        CollectStats(state.root, seen, state.stats, 0);
    } catch (const std::exception& ex) {
        state.root = nullptr;
        state.error = ex.what();
        state.stats = {};
    }
}

void ApplyTheme(float scale) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 8.0f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.41f, 0.48f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.52f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.18f, 0.33f, 0.56f, 0.65f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.40f, 0.72f, 0.85f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.23f, 0.50f, 0.85f, 1.00f);

    if (scale > 0.99f)
        style.ScaleAllSizes(scale);
}

void BuildInitialDockLayout(const ImGuiID dockspaceId) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, vp->WorkSize);

    ImGuiID dockLeft = dockspaceId;
    ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Right, 0.33f, nullptr, &dockLeft);
    ImGuiID dockRightBottom = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.35f, nullptr, &dockRight);

    ImGui::DockBuilderDockWindow("Expression Tree", dockLeft);
    ImGui::DockBuilderDockWindow("Node Details", dockRight);
    ImGui::DockBuilderDockWindow("Graph Stats", dockRightBottom);
    ImGui::DockBuilderFinish(dockspaceId);
}

void DrawToolbar(ExplorerState& state) {
    if (ImGui::Button("📄 Parse"))
        ParseExpression(state);
    ImGui::SameLine();
    if (ImGui::Button("➕ Expand All"))
        state.requestExpandAll = true;
    ImGui::SameLine();
    if (ImGui::Button("➖ Collapse All"))
        state.requestCollapseAll = true;
    ImGui::SameLine();
    ImGui::TextDisabled("DPI x%.2f", state.uiScale);
}

void DrawTreeNode(const Expr* node, ExplorerState& state, std::unordered_set<uint32_t>& seenAny,
                  std::vector<uint32_t>& path, int depth) {
    if (node == nullptr)
        return;

    const bool isSharedRef = !seenAny.insert(node->id.value()).second;
    const bool isCycle = std::find(path.begin(), path.end(), node->id.value()) != path.end();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (node->inputs.empty() || isCycle)
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (state.selected == node)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (state.requestExpandAll)
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    else if (state.requestCollapseAll)
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);

    const std::string label = NodeLabel(node, state.names);
    std::ostringstream itemId;
    itemId << "##node_" << node->id.value() << "_d" << depth << "_p" << path.size();

    const ImVec4 c = OpColor(node->op);
    ImGui::PushStyleColor(ImGuiCol_Text, c);
    const bool open = ImGui::TreeNodeEx(reinterpret_cast<const void*>(itemId.str().c_str()), flags, "%s %s%s%s",
                                        OpIcon(node->op), label.c_str(),
                                        isSharedRef ? " (ref)" : "", isCycle ? " (cycle)" : "");
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked()) {
        state.selected = node;
        state.selectedDepth = depth;
    }

    if (open) {
        if (!isCycle) {
            path.push_back(node->id.value());
            for (const Expr* child : node->inputs)
                DrawTreeNode(child, state, seenAny, path, depth + 1);
            path.pop_back();
        }
        ImGui::TreePop();
    }
}

void DrawExpressionTree(ExplorerState& state) {
    ImGui::Begin("Expression Tree");

    DrawToolbar(state);
    ImGui::Separator();

    ImGui::TextUnformatted("Expression:");
    ImGui::InputTextMultiline("##expr_input", state.input.data(), state.input.size(), ImVec2(-FLT_MIN, 120));

    if (!state.error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 110, 110, 255));
        ImGui::TextWrapped("Parse error: %s", state.error.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    if (state.root == nullptr) {
        ImGui::TextDisabled("No expression loaded.");
    } else {
        std::unordered_set<uint32_t> seenAny;
        std::vector<uint32_t> path;
        DrawTreeNode(state.root, state, seenAny, path, 0);
    }

    state.requestExpandAll = false;
    state.requestCollapseAll = false;

    ImGui::End();
}

void DrawNodeDetails(const ExplorerState& state) {
    ImGui::Begin("Node Details");

    if (state.selected == nullptr) {
        ImGui::TextDisabled("Select a node from the tree.");
        ImGui::End();
        return;
    }

    ImGui::Text("General");
    ImGui::Separator();
    ImGui::Text("ID       : n%u", state.selected->id.value());
    ImGui::Text("Type     : %s", OpName(state.selected->op));
    ImGui::Text("Label    : %s", NodeLabel(state.selected, state.names).c_str());
    ImGui::Text("Children : %zu", state.selected->inputs.size());
    ImGui::Text("Depth    : %d", state.selectedDepth);

    ImGui::Spacing();
    ImGui::Text("Preview");
    ImGui::Separator();
    const auto text = BitFlow::IO::ToString(state.selected, state.names);
    ImGui::PushStyleColor(ImGuiCol_Text, OpColor(state.selected->op));
    ImGui::TextWrapped("%s", text.c_str());
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Text("Node Legend");
    ImGui::Separator();

    const std::array<OpType, 6> legendOps = {OpType::Var, OpType::Const, OpType::Add, OpType::Xor, OpType::RotR, OpType::Ch};
    for (OpType op : legendOps) {
        ImGui::PushStyleColor(ImGuiCol_Text, OpColor(op));
        ImGui::Text("%s %s", OpIcon(op), OpName(op));
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

void DrawStats(const ExplorerState& state) {
    ImGui::Begin("Graph Stats");

    ImGui::Text("Nodes visited : %d", state.stats.totalNodes);
    ImGui::Text("Unique nodes  : %d", state.stats.uniqueNodes);
    ImGui::Text("Shared refs   : %d", state.stats.sharedRefs);
    ImGui::Text("Max depth     : %d", state.stats.maxDepth);

    ImGui::End();
}

} // namespace

int main() {
    if (!glfwInit())
        return 1;

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "BitFlow - Expression Explorer", nullptr, nullptr);
    if (!window)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    float sx = 1.0f;
    float sy = 1.0f;
    glfwGetWindowContentScale(window, &sx, &sy);
    const float dpiScale = std::max(1.0f, std::max(sx, sy));

    io.FontGlobalScale = dpiScale;
    ApplyTheme(dpiScale);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    ExplorerState state;
    state.uiScale = dpiScale;

    {
        const std::string def =
            "e0 + 3921009573 + (rotr(u, 6) ^ rotr(u, 11) ^ rotr(u, 25)) + ch(a, u, c)";
        const size_t n = std::min(def.size(), state.input.size() - 1);
        std::copy(def.begin(), def.begin() + static_cast<std::ptrdiff_t>(n), state.input.begin());
        state.input[n] = '\0';
    }
    ParseExpression(state);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("RootDockHost", nullptr, hostFlags);
        ImGui::PopStyleVar(2);

        const ImGuiID dockspaceId = ImGui::GetID("BitFlowExprDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);

        if (!state.dockLayoutBuilt) {
            BuildInitialDockLayout(dockspaceId);
            state.dockLayoutBuilt = true;
        }

        DrawExpressionTree(state);
        DrawNodeDetails(state);
        DrawStats(state);

        ImGui::End();

        ImGui::Render();
        int w = 0;
        int h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
