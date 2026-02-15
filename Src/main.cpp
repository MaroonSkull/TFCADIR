#include <Controller/OpenGL/ImGUI.hpp>
#include <Logging/LoggerManager.hpp>
#include <Model/FlatFigure.hpp>
#include <View/OpenGL/ImGUI.hpp>

#include <iostream>
#include <spdlog/spdlog.h>

// #include <swogl/swogl.h>

void handle_eptr(std::exception_ptr eptr) // passing by value is OK
{
  try {
    if (eptr)
      std::rethrow_exception(eptr);
    spdlog::error("Caught nullptr exception!\r\n");
  } catch (const std::exception &e) {
    spdlog::error("Caught std::exception: '{}'\r\n", e.what());
  } catch (const char *msg) {
    spdlog::error("Caught C-string exception: '{}'\r\n",
                  msg ? msg : "EMPTY STRING");
  } catch (...) {
    spdlog::error("Unknown exception!\r\n");
  }
}

int main() {
  // Initialize logging system
  if (!TFCADIR::Logging::LoggerManager::instance().initialize(
          "config/logging.yaml")) {
    std::cerr << "Failed to initialize logging system" << std::endl;
    return 1;
  }

  // Log application startup
  spdlog::info("TFCADIR application starting");

  try {
    auto Model = std::make_shared<model::FlatFigures>();
    auto Controller = std::shared_ptr<controller::IController>(
        new controller::OpenglImguiController(Model));
    auto View = std::shared_ptr<IView>(new OpenglImguiView(Model, Controller));

    // Hehe, i don't use swogle here, cause swogl is not ready yet :death:
    // spdlog::info("swogl::add(5, 10) = {}", swogl::add(5, 10));

    while (!View->shouldClose()) {
      // catch all events
      glfw::pollEvents();

      View->draw();
    }
  } catch (...) {
    handle_eptr(std::current_exception());
  }

  // Log application shutdown
  spdlog::info("TFCADIR application shutting down");

  // Shutdown logging system before exit
  TFCADIR::Logging::LoggerManager::instance().shutdown();

  return 0;
}
