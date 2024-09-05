#pragma once
#include <glfwpp/glfwpp.h>
#include <imgui.h>
#include <imgui_internal.h> // для docking

class GUI {
private:
	ImGuiDockNodeFlags dockFlags_ = ImGuiDockNodeFlags_None;
	ImGuiID dockId_{}; // id родительского окна, к которому будем цеплять все прочие панели
	ImGuiID dockIdTools, dockIdLog, dockIdMouse;
	ImVec2 mouseOverlayPosition_;
	ImVec2 mousePositionAbsolute_;
	ImTextureID texture;
	ImGuiWindowFlags canvasFlags_ = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;
	bool isCanvasHovered_{};
	ImVec2 mousePositionRelative_{};
	float momentWheel_{};

	bool circleSegmentsOverride = false;
	static const int circleSegmentsOverride_v_default = 15;
	int circleSegmentsOverride_v = circleSegmentsOverride_v_default;

	bool curveSegmentsOverride = false;
	static const int curveSegmentsOverride_v_default = 15;
	int curveSegmentsOverride_v = curveSegmentsOverride_v_default;


	void ShowMainMenuBar();
	void ShowDockSpace();
	void ShowLog();
	void ShowSidePanel();
	ImVec2 ShowCanvas(ImTextureID renderTexture);
	// void ShowListPanel()
	void ShowSimpleOverlay(); // пусть плавает за мышью и показывает её координаты, если они в пределах канваса
public:
	std::tuple<ImVec2, float, std::optional<ImVec2>> DrawGUI(ImTextureID renderTexture);
};