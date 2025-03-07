#include <Model/FlatFigure.hpp>
#include <View/OpenGL/ImGUI.hpp>
#include <Controller/OpenGL/ImGUI.hpp>

#include <spdlog/spdlog.h>

// #include <swogl/swogl.h>


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
		spdlog::set_level(spdlog::level::debug); // Set the spdlog level to debug

		auto Model = std::make_shared<model::FlatFigures>();
		auto Controller = std::shared_ptr<controller::IController>(new controller::OpenglImguiController(Model));
		auto View = std::shared_ptr<IView>(new OpenglImguiView(Model, Controller));

		// Hehe, i don't use swogle here, cause swogl is not ready yet :death:
		// spdlog::info("swogl::add(5, 10) = {}", swogl::add(5, 10));

		while (!View->shouldClose()) {
			// отлавливаем все события
			glfw::pollEvents();

			View->draw();
		}
	}
	catch (...) {
		handle_eptr(std::current_exception());
	}

	// Under VisualStudio, this must be called before main finishes to workaround a known VS issue
	spdlog::drop_all();

	return 0;
}
