#pragma once

#include "ICommand.hpp"
#include <Model/FlatFigure.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for duplicating figures with an offset
 *
 * DuplicateFigureCommand encapsulates creating a copy of an existing figure
 * with a positional offset. Stores the new figure ID for undo functionality
 * and supports JSON serialization for persistence.
 */
class DuplicateFigureCommand : public Commands::ICommand {
public:
  /**
   * @brief Constructs a command to duplicate a figure
   * @param model Reference to the FlatFigures model
   * @param sourceFigureId The ID of the figure to duplicate
   * @param offset The positional offset for the duplicated figure
   */
  DuplicateFigureCommand(model::FlatFigures &model, uint32_t sourceFigureId,
                         glm::vec3 offset);

  /**
   * @brief Destructor
   */
  ~DuplicateFigureCommand() override = default;

  void execute() override;
  void undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the FlatFigures model
  model::FlatFigures &model_;

  /// The ID of the source figure to duplicate
  uint32_t sourceFigureId_;

  /// The ID of the newly created figure (assigned during execute)
  uint32_t newFigureId_;

  /// The positional offset for the duplicated figure
  glm::vec3 offset_;

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
