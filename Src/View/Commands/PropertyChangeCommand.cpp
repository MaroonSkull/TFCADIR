#include "PropertyChangeCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <variant>

namespace view {

PropertyChangeCommand::PropertyChangeCommand(UIFSMAdapter &fsmAdapter,
                                             uint32_t figureId,
                                             const std::string &propertyPath,
                                             PropertyValue newValue)
    : fsmAdapter_(fsmAdapter), figureId_(figureId), propertyPath_(propertyPath),
      newValue_(newValue), previousValue_(0), executed_(false) {}

bool PropertyChangeCommand::execute() {
  if (executed_) {
    spdlog::warn("PropertyChangeCommand: Already executed");
    return false;
  }

  try {
    // NOTE: Property change command is not yet implemented
    // This requires adding UIFSMAdapter methods:
    // - getFigurePropertyInt(figureId, propertyPath)
    // - getFigurePropertyDouble(figureId, propertyPath)
    // - getFigurePropertyBool(figureId, propertyPath)
    // - getFigurePropertyString(figureId, propertyPath)
    // - updateFigureProperty(figureId, propertyPath, value)
    // For now, this command is disabled.
    spdlog::error("PropertyChangeCommand: Property change not yet implemented. "
                  "Requires UIFSMAdapter extensions for property path system.");
    return false;
  } catch (const std::exception &e) {
    spdlog::error("PropertyChangeCommand: Failed to change property: {}",
                  e.what());
    return false;
  }
}

bool PropertyChangeCommand::undo() {
  if (!executed_) {
    spdlog::warn("PropertyChangeCommand: Not executed, cannot undo");
    return false;
  }

  try {
    // Restore previous value
    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, int>) {
            fsmAdapter_.updateFigureProperty(figureId_, propertyPath_,
                                             std::get<int>(previousValue_));
          } else if constexpr (std::is_same_v<T, double>) {
            fsmAdapter_.updateFigureProperty(figureId_, propertyPath_,
                                             std::get<double>(previousValue_));
          } else if constexpr (std::is_same_v<T, bool>) {
            fsmAdapter_.updateFigureProperty(figureId_, propertyPath_,
                                             std::get<bool>(previousValue_));
          } else if constexpr (std::is_same_v<T, std::string>) {
            fsmAdapter_.updateFigureProperty(
                figureId_, propertyPath_,
                std::get<std::string>(previousValue_));
          }
        },
        previousValue_);

    executed_ = false;
    spdlog::info("PropertyChangeCommand: Restored figure {} property {}",
                 figureId_, propertyPath_);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("PropertyChangeCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string PropertyChangeCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Change Property '" << propertyPath_ << "' of Figure #" << figureId_
      << " to " << valueToString(newValue_);
  return oss.str();
}

std::string PropertyChangeCommand::getType() const { return "PropertyChange"; }

std::vector<uint32_t> PropertyChangeCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string PropertyChangeCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "PropertyChange";
  j["figureId"] = figureId_;
  j["propertyPath"] = propertyPath_;
  j["newValue"] = valueToJson(newValue_);
  j["previousValue"] = valueToJson(previousValue_);
  j["executed"] = executed_;

  return j.dump();
}

nlohmann::json
PropertyChangeCommand::valueToJson(const PropertyValue &value) const {
  nlohmann::json j;
  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
          j = arg;
        } else if constexpr (std::is_same_v<T, double>) {
          j = arg;
        } else if constexpr (std::is_same_v<T, bool>) {
          j = arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
          j = arg;
        }
      },
      value);
  return j;
}

std::string
PropertyChangeCommand::valueToString(const PropertyValue &value) const {
  std::ostringstream oss;
  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
          oss << arg;
        } else if constexpr (std::is_same_v<T, double>) {
          oss << arg;
        } else if constexpr (std::is_same_v<T, bool>) {
          oss << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, std::string>) {
          oss << "\"" << arg << "\"";
        }
      },
      value);
  return oss.str();
}

} // namespace view
