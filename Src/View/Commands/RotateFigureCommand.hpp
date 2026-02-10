#pragma once

#include "ICommand.hpp"
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for rotating figures by a specified angle
 *
 * RotateFigureCommand encapsulates the rotation of a figure around its origin.
 * Uses UIFSMAdapter to access and modify figure properties via the property
 * path system. Supports JSON serialization for persistence.
 */
class RotateFigureCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to rotate a figure
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param figureId The ID of the figure to rotate
   * @param angleDegrees The rotation angle in degrees
   */
  RotateFigureCommand(UIFSMAdapter &fsmAdapter, uint32_t figureId,
                      float angleDegrees);

  /**
   * @brief Destructor
   */
  ~RotateFigureCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter for property access
  UIFSMAdapter &fsmAdapter_;

  /// The ID of the figure to rotate
  uint32_t figureId_;

  /// The rotation angle in degrees
  float angleDegrees_;

  /// The previous angle before rotation (for undo)
  float previousAngle_;

  /// Tracks whether the command has been executed
  bool executed_;
};

} // namespace view
