#include "CommandFactory.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace view {

void CommandFactory::registerCommand(
    const std::string &type,
    std::function<std::unique_ptr<ICommand>(const nlohmann::json &)> factory) {
  factories_[type] = std::move(factory);
}

std::unique_ptr<ICommand>
CommandFactory::deserialize(const std::string &jsonStr) const {
  try {
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    std::string type = j["type"];

    auto it = factories_.find(type);
    if (it == factories_.end()) {
      spdlog::error("CommandFactory: Unknown command type '{}'", type);
      return nullptr;
    }

    return it->second(j);
  } catch (const std::exception &e) {
    spdlog::error("CommandFactory: Failed to deserialize command: {}",
                  e.what());
    return nullptr;
  }
}

bool CommandFactory::isRegistered(const std::string &type) const {
  return factories_.find(type) != factories_.end();
}

} // namespace view
