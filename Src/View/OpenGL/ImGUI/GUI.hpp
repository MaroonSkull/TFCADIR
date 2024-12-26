#pragma once

#include <glfwpp/glfwpp.h>
#include <imgui.h>
#include <imgui_internal.h> // для docking



class GUI {
private:
	ImGuiDockNodeFlags dockFlags_{ ImGuiDockNodeFlags_None };
	ImGuiID dockId_{}; // id родительского окна, к которому будем цеплять все прочие панели
	ImGuiID dockIdTools_{};
	ImGuiID dockIdLog_{};
	ImGuiID dockIdMouse_{};
	ImVec2 mouseOverlayPosition_{};
	ImVec2 mousePositionAbsolute_{};
	ImTextureID textureId_{};
	ImGuiWindowFlags canvasFlags_{ ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs };
	bool isCanvasHovered_{}; // current state of canvas
	ImVec2 mousePositionRelative_{}; // mouse coords in canvas space
	float momentWheel_{}; // momentum speed of scroll

	bool circleSegmentsOverride_{ false };
	static const int circleSegmentsOverride_v_default = 15;
	int circleSegmentsOverride_v_{ circleSegmentsOverride_v_default };

	bool curveSegmentsOverride_{ false };
	static const int curveSegmentsOverride_v_default = 15;
	int curveSegmentsOverride_v_{ curveSegmentsOverride_v_default };


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