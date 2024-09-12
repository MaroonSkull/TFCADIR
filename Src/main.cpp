#include <Model/FlatFigure.hpp>
#include <View/OpenGL/ImGUI.hpp>
#include <Controller/OpenGL/ImGUI.hpp>

#include <spdlog/spdlog.h>


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
		auto sp_model = std::make_shared<model::FlatFigures>();
		auto sp_controller = std::shared_ptr<IController>(new OpenglImguiController(sp_model));
		auto sp_view = std::shared_ptr<IView>(new OpenglImguiView(sp_model, sp_controller));

		while (!sp_view->shouldClose()) {
			// отлавливаем все события
			glfw::pollEvents();

			sp_view->draw();
		}
	}
	catch (...) {
		handle_eptr(std::current_exception());
	}

	// Under VisualStudio, this must be called before main finishes to workaround a known VS issue
	spdlog::drop_all();
	return 0;
}
