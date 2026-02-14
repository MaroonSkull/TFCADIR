#pragma once

#include "ICommand.hpp"
#include <Model/FlatFigure.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for moving figures by a delta offset
 *
 * MoveFigureCommand encapsulates the translation of a figure in 3D space.
 * Stores the previous position for undo functionality and supports
 * JSON serialization for persistence.
 */
class MoveFigureCommand : public Commands::ICommand {
public:
  /**
   * @brief Constructs a command to move a figure
   * @param model Reference to the FlatFigures model
   * @param figureId The ID of the figure to move
   * @param delta The translation offset in 3D space
   */
  MoveFigureCommand(model::FlatFigures &model, uint32_t figureId,
                    glm::vec3 delta);

  /**
   * @brief Destructor
   */
  ~MoveFigureCommand() override = default;

  void execute() override;
  void undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the FlatFigures model
  model::FlatFigures &model_;

  /// The ID of the figure to move
  uint32_t figureId_;

  /// The translation delta to apply
  glm::vec3 delta_;

  /// The previous position before moving (for undo)
  glm::vec3 previousPosition_;

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
