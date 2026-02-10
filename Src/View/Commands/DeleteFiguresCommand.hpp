#pragma once

#include "ICommand.hpp"
#include <Model/FlatFigure.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for deleting figures from the model
 *
 * DeleteFiguresCommand encapsulates removing figures from the FlatFigures
 * model. Stores the deleted figure for undo functionality and supports JSON
 * serialization.
 */
class DeleteFiguresCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to delete a figure
   * @param model Reference to the FlatFigures model
   * @param figureId The ID of the figure to delete
   */
  DeleteFiguresCommand(model::FlatFigures &model, uint32_t figureId);

  /**
   * @brief Destructor
   */
  ~DeleteFiguresCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the FlatFigures model
  model::FlatFigures &model_;

  /// The ID of the figure to delete
  uint32_t figureId_;

  /// Shared pointer to the deleted figure (for undo)
  std::shared_ptr<model::IFigure> deletedFigure_;

  /// Tracks whether the command has been executed
  bool executed_;
};

} // namespace view
