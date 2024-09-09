#include <spdlog/spdlog.h>

#include <Model/FlatFigure.hpp>
#include <View/OpenGL/ImGUI.hpp>
#include <Controller/OpenGL/ImGUI.hpp>
void handle_eptr(std::exception_ptr eptr) // passing by value is OK
{
    try {
        if (eptr)
            std::rethrow_exception(eptr);
		spdlog::error("Caught nullptr exception!\r\n");
    }
    catch(const std::exception& e) {
        spdlog::error("Caught std::exception: '{}'\r\n", e.what());
    }
	catch (const char* msg) {
		spdlog::error("Caught C-string exception: '{}'\r\n",
			msg ? msg : "EMPTY STRING"
		);
	}
	catch (...) {
		spdlog::error("Unknown exception!\r\n");
	}
}

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
	catch (...) {
		handle_eptr(std::current_exception());
	}

	// Under VisualStudio, this must be called before main finishes to workaround a known VS issue
	spdlog::drop_all();
	return 0;
}
