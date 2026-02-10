#pragma once

#include "ICommand.hpp"
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

namespace view {

/**
 * @brief Command for changing figure properties via property path
 *
 * PropertyChangeCommand encapsulates modifying a specific property of a figure
 * using the UIFSMAdapter property path system. Stores the previous value for
 * undo functionality and supports JSON serialization.
 */
class PropertyChangeCommand : public ICommand {
public:
  /**
   * @brief Variant type for property values
   */
  using PropertyValue = std::variant<int, double, bool, std::string>;

  /**
   * @brief Constructs a command to change a figure property
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param figureId The ID of the figure
   * @param propertyPath The property path (e.g., "position.x", "color.r")
   * @param newValue The new value to set
   */
  PropertyChangeCommand(UIFSMAdapter &fsmAdapter, uint32_t figureId,
                        const std::string &propertyPath,
                        PropertyValue newValue);

  /**
   * @brief Destructor
   */
  ~PropertyChangeCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter
  UIFSMAdapter &fsmAdapter_;

  /// The ID of the figure
  uint32_t figureId_;

  /// The property path to modify
  std::string propertyPath_;

  /// The new value to set
  PropertyValue newValue_;

  /// The previous value (for undo)
  PropertyValue previousValue_;

  /// Tracks whether the command has been executed
  bool executed_;

  /**
   * @brief Helper to convert a PropertyValue to JSON
   * @param value The value to convert
   * @return JSON representation
   */
  nlohmann::json valueToJson(const PropertyValue &value) const;

  /**
   * @brief Helper to convert PropertyValue to string for description
   * @param value The value to convert
   * @return String representation
   */
  std::string valueToString(const PropertyValue &value) const;
};

} // namespace view
