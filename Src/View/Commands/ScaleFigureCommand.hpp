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
 * @brief Command for scaling figures by a specified factor
 *
 * ScaleFigureCommand encapsulates the scaling of a figure in 3D space.
 * Uses UIFSMAdapter to access and modify figure properties via the property
 * path system. Supports JSON serialization for persistence.
 */
class ScaleFigureCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to scale a figure
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param figureId The ID of the figure to scale
   * @param scaleFactor The scaling factor in 3D space (x, y, z)
   */
  ScaleFigureCommand(UIFSMAdapter &fsmAdapter, uint32_t figureId,
                     glm::vec3 scaleFactor);

  /**
   * @brief Destructor
   */
  ~ScaleFigureCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter for property access
  UIFSMAdapter &fsmAdapter_;

  /// The ID of the figure to scale
  uint32_t figureId_;

  /// The scaling factor to apply
  glm::vec3 scaleFactor_;

  /// The previous scale before scaling (for undo)
  glm::vec3 previousScale_;

  /// Tracks whether the command has been executed
  bool executed_;

  /**
   * @brief Helper to convert a glm::vec3 to JSON object
   * @param vec The vector to convert
   * @return JSON object with x, y, z fields
   */
  nlohmann::json vec3ToJson(const glm::vec3 &vec) const;
};

} // namespace view
