#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cfloat>
#include <exception>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

using BitFlow::Core::AST::Expr;
using BitFlow::Core::AST::OpType;

struct ExplorerState {
    std::array<char, 4096> input{};
    std::string error;
    std::unordered_map<uint32_t, std::string> names;
    Expr* root = nullptr;

    const Expr* selected = nullptr;
    bool requestExpandAll = false;
    bool requestCollapseAll = false;
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

std::string NodeTitle(const Expr* node, const std::unordered_map<uint32_t, std::string>& names) {
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

void ParseExpression(ExplorerState& state) {
    try {
        auto parsed = BitFlow::IO::Parse(state.input.data());
        state.root = parsed.root;
        state.names = std::move(parsed.idToName);
        state.error.clear();
        state.selected = state.root;
    } catch (const std::exception& ex) {
        state.root = nullptr;
        state.error = ex.what();
    }
}

struct GraphStats {
    int totalNodes = 0;
    int uniqueNodes = 0;
    int sharedRefs = 0;
    int maxDepth = 0;
};

void CollectStats(const Expr* node, std::unordered_set<uint32_t>& seen, GraphStats& stats, int depth) {
    if (node == nullptr)
        return;

    stats.totalNodes++;
    stats.maxDepth = std::max(stats.maxDepth, depth);

    const auto [it, inserted] = seen.insert(node->id.value());
    if (inserted) {
        stats.uniqueNodes++;
    } else {
        stats.sharedRefs++;
        return;
    }

    for (const Expr* child : node->inputs)
        CollectStats(child, seen, stats, depth + 1);
}

void DrawTreeNode(const Expr* node, ExplorerState& state, std::unordered_set<uint32_t>& rendered, int depth) {
    if (node == nullptr)
        return;

    const bool firstTime = rendered.insert(node->id.value()).second;
    const bool hasChildren = !node->inputs.empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (state.selected == node)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (state.requestExpandAll)
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    else if (state.requestCollapseAll)
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);

    std::ostringstream label;
    label << NodeTitle(node, state.names) << " [" << OpName(node->op) << "]";
    if (!firstTime)
        label << " (ref)";
    label << "##n" << node->id.value() << "_d" << depth;

    const bool open = ImGui::TreeNodeEx(reinterpret_cast<const void*>(node), flags, "%s", label.str().c_str());
    if (ImGui::IsItemClicked())
        state.selected = node;

    if (open) {
        if (firstTime) {
            for (const Expr* child : node->inputs)
                DrawTreeNode(child, state, rendered, depth + 1);
        }
        ImGui::TreePop();
    }
}

void DrawTreePanel(ExplorerState& state) {
    ImGui::Begin("Expression Tree");

    if (ImGui::Button("Parse"))
        ParseExpression(state);
    ImGui::SameLine();
    if (ImGui::Button("Expand All"))
        state.requestExpandAll = true;
    ImGui::SameLine();
    if (ImGui::Button("Collapse All"))
        state.requestCollapseAll = true;

    ImGui::Separator();

    ImGui::TextUnformatted("Expression:");
    ImGui::InputTextMultiline("##expr", state.input.data(), state.input.size(), ImVec2(-FLT_MIN, 96));

    if (!state.error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(240, 96, 96, 255));
        ImGui::TextWrapped("Parse error: %s", state.error.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    if (state.root != nullptr) {
        std::unordered_set<uint32_t> rendered;
        DrawTreeNode(state.root, state, rendered, 0);
    } else {
        ImGui::TextDisabled("No expression parsed yet.");
    }

    state.requestExpandAll = false;
    state.requestCollapseAll = false;

    ImGui::End();
}

void DrawDetailsPanel(const ExplorerState& state) {
    ImGui::Begin("Node Details");

    if (state.selected == nullptr) {
        ImGui::TextDisabled("Select a node from the tree.");
        ImGui::End();
        return;
    }

    ImGui::Text("ID: n%u", state.selected->id.value());
    ImGui::Text("Type: %s", OpName(state.selected->op));
    ImGui::Text("Children: %zu", state.selected->inputs.size());

    if (state.selected->op == OpType::Const)
        ImGui::Text("Const value: %u", state.selected->constValue);

    if (state.selected->op == OpType::Var) {
        const auto it = state.names.find(state.selected->id.value());
        ImGui::Text("Name: %s", it != state.names.end() ? it->second.c_str() : "<unnamed>");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Preview:");
    const std::string exprText = BitFlow::IO::ToString(state.selected, state.names);
    ImGui::TextWrapped("%s", exprText.c_str());

    ImGui::End();
}

void DrawStatsPanel(const ExplorerState& state) {
    ImGui::Begin("Graph Stats");

    if (state.root == nullptr) {
        ImGui::TextDisabled("No stats available.");
        ImGui::End();
        return;
    }

    GraphStats stats{};
    std::unordered_set<uint32_t> seen;
    CollectStats(state.root, seen, stats, 0);

    ImGui::Text("Nodes visited : %d", stats.totalNodes);
    ImGui::Text("Unique nodes  : %d", stats.uniqueNodes);
    ImGui::Text("Shared refs   : %d", stats.sharedRefs);
    ImGui::Text("Max depth     : %d", stats.maxDepth);

    ImGui::End();
}

} // namespace

int main() {
    if (!glfwInit())
        return 1;

#if defined(__APPLE__)
    const char* glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glslVersion = "#version 130";
#endif

    GLFWwindow* window = glfwCreateWindow(1600, 920, "BitFlow - Expression Explorer", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    ExplorerState state;
    {
        const std::string defaultExpr = "h0 + 0x428a2f98 + (rotr(e0, 6) ^ rotr(e0, 11) ^ rotr(e0, 25)) + ch(e0, f0, g0)";
        std::copy(defaultExpr.begin(), defaultExpr.end(), state.input.begin());
        state.input[defaultExpr.size()] = '\0';
    }
    ParseExpression(state);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        hostFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("BitFlowDockHost", nullptr, hostFlags);
        ImGui::PopStyleVar(2);

        ImGuiID dockspaceID = ImGui::GetID("BitFlowExprDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        DrawTreePanel(state);
        DrawDetailsPanel(state);
        DrawStatsPanel(state);

        ImGui::End();

        ImGui::Render();
        int displayW = 0;
        int displayH = 0;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.10f, 0.11f, 0.12f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
