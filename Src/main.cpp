#include <spdlog/spdlog.h>

#include <Model/FlatFigure.hpp>
#include <View/OpenGL/ImGUI.hpp>
#include <Controller/OpenGL/ImGUI.hpp>

int main() {
	try {
		model::FlatFigures* pModel = new model::FlatFigures();
		IController *pController = new OpenglImguiController(pModel);
		IView* pView = new OpenglImguiView(pModel, pController);

		while (!pView->shouldClose()) {
			// отлавливаем все события
			glfw::pollEvents();

			pView->draw();
		}
	}
	catch (const std::runtime_error& e) {
		spdlog::error("std::runtime_error: {}", e.what());
	}
	catch (const std::exception& e) {
		spdlog::error("std::exception: {}", e.what());
	}
	catch (const char* e) {
		spdlog::error(e, "\r\n");
	}
	catch (...) {
		spdlog::error("Unknown exception!");
	}
	// Under VisualStudio, this must be called before main finishes to workaround a known VS issue
	spdlog::drop_all();
	return 0;
}
