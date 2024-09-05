#include <GUI.hpp>
using namespace ImGui;

void GUI::ShowMainMenuBar() {
    if(BeginMainMenuBar()) {
        if(BeginMenu("Menu")) {
            if(BeginMenu("Examples")) {
                if(MenuItem("1")) {}
                if(MenuItem("2")) {}
                if(MenuItem("3")) {}
                EndMenu();
            }
            EndMenu();
        }
        if(BeginMenu("Edit")) {
            if(MenuItem("Undo", "CTRL+Z")) {}
            if(MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            EndMenu();
        }
        if(BeginMenu("View")) {
            if(MenuItem("Grid")) {}
            if(MenuItem("Sized", NULL, false, false)) {}  // Disabled item
            Separator();
            
            if(Checkbox("##circlesegmentoverride", &circleSegmentsOverride))
                circleSegmentsOverride_v = circleSegmentsOverride_v_default;
            SameLine(0.0f, GetStyle().ItemInnerSpacing.x);
            circleSegmentsOverride |= SliderInt("Circle segments override", &circleSegmentsOverride_v, 3, 40);
            if(Checkbox("##curvessegmentoverride", &curveSegmentsOverride))
                curveSegmentsOverride_v = curveSegmentsOverride_v_default;
            SameLine(0.0f, GetStyle().ItemInnerSpacing.x);
            curveSegmentsOverride |= SliderInt("Curves segments override", &curveSegmentsOverride_v, 3, 40);
            EndMenu();
        }
        EndMainMenuBar();
    }
}

void GUI::ShowDockSpace() {
    dockId_ = DockSpaceOverViewport(0, GetMainViewport()); // move inside if
    static bool init = true;
    if(init) {
        init = false;
        DockBuilderRemoveNode(dockId_);
        dockId_ = DockBuilderAddNode(dockId_, dockFlags_);
        DockBuilderSetNodeSize(dockId_, GetMainViewport()->Size);

        dockIdLog = DockBuilderSplitNode(dockId_, ImGuiDir_Down, 0.15f, nullptr, &dockId_);
        dockIdTools = DockBuilderSplitNode(dockId_, ImGuiDir_Right, 0.25f, nullptr, &dockId_);

        DockBuilderDockWindow("Canvas", dockId_);
        DockBuilderDockWindow("Tools", dockIdTools);
        DockBuilderDockWindow("Log", dockIdLog);
        DockBuilderDockWindow("Mouse coords", dockIdMouse);

        DockBuilderFinish(dockId_);
    }
}

void GUI::ShowLog() {
    if(Begin("Log")) {
        Text("Just logs here");
    }
    End();
}

void GUI::ShowSidePanel() {
    if(Begin("Tools")) {
        Text("Side panel with tools");
    }
    End();
}

ImVec2 GUI::ShowCanvas(ImTextureID renderTexture) {
    ImVec2 canvasSize{};
    ImVec2 screenPositionAbsolute{};
    if (Begin("Canvas", nullptr, canvasFlags_)) {
        // Using a Child allow to fill all the space of the window.
        // It also alows customization
        BeginChild("##Canvas", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoMove);
        // Разрешаем перехватывать событие прокрутки колесика
        SetItemKeyOwner(ImGuiKey_MouseWheelY);
        momentWheel_ = GetIO().MouseWheel;
        // Get the size of the child (i.e. the whole draw size of the windows).
        canvasSize = GetWindowSize();

        // Because I use the texture from OpenGL, I need to invert the V from the UV.
        Image(renderTexture, canvasSize, ImVec2(0, 1), ImVec2(1, 0));
        isCanvasHovered_ = IsItemHovered();
        mousePositionAbsolute_ = GetMousePos();
        screenPositionAbsolute = GetItemRectMin();
        mousePositionRelative_ = ImVec2(mousePositionAbsolute_.x - screenPositionAbsolute.x, mousePositionAbsolute_.y - screenPositionAbsolute.y);

        EndChild();
    }
    End();
    return canvasSize;
}

void GUI::ShowSimpleOverlay() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav;
    SetNextWindowBgAlpha(0.35f); // Transparent background
    ImVec2 overlayPos = mousePositionAbsolute_;
    overlayPos.x += 16;
    overlayPos.y += 24;
    SetNextWindowPos(overlayPos, ImGuiCond_Always);
    SetNextWindowViewport(GetMainViewport()->ID);

    if(Begin("Mouse coords", nullptr, window_flags)) {
        if(IsMousePosValid())
            Text("%.1f,%.1f", mousePositionRelative_.x, mousePositionRelative_.y);
        else
            Text("<invalid>");
    }
    End();
}

std::tuple<ImVec2, float, std::optional<ImVec2>> GUI::DrawGUI(ImTextureID renderTexture) {
    NewFrame();
    ShowMainMenuBar();
	ShowDockSpace();
	ShowLog();
	ShowSidePanel();
	auto canvasSize = ShowCanvas(renderTexture);
	if (isCanvasHovered_) ShowSimpleOverlay();

	ShowDemoWindow();
    Render();

    return (isCanvasHovered_) ?
        std::tuple<ImVec2, float, std::optional<ImVec2>>(canvasSize, momentWheel_, mousePositionRelative_) :
        std::tuple<ImVec2, float, std::optional<ImVec2>>(canvasSize, momentWheel_, std::nullopt);
}
