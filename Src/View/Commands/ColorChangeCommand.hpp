#pragma once

#include "ICommand.hpp"
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for changing the color of figures
 *
 * ColorChangeCommand encapsulates changing a figure's color property.
 * Uses UIFSMAdapter to access and modify figure properties via the property
 * path system. Supports JSON serialization for persistence.
 */
class ColorChangeCommand : public Commands::ICommand {
public:
  /**
   * @brief Constructs a command to change a figure's color
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param figureId The ID of the figure to modify
   * @param newColor The new color (RGBA)
   */
  ColorChangeCommand(UIFSMAdapter &fsmAdapter, uint32_t figureId,
                     glm::vec4 newColor);

  /**
   * @brief Destructor
   */
  ~ColorChangeCommand() override = default;

  void execute() override;
  void undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter for property access
  UIFSMAdapter &fsmAdapter_;

  /// The ID of the figure to modify
  uint32_t figureId_;

  /// The new color to apply
  glm::vec4 newColor_;

  /// The previous color before change (for undo)
  glm::vec4 previousColor_;

  /// Tracks whether the command has been executed
  bool executed_;

  /**
   * @brief Helper to convert a glm::vec4 to JSON object
   * @param vec The vector to convert
   * @return JSON object with r, g, b, a fields
   */
  nlohmann::json vec4ToJson(const glm::vec4 &vec) const;
};

} // namespace view
