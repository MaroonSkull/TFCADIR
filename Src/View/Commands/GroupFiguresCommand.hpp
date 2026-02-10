#pragma once

#include "ICommand.hpp"
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for grouping multiple figures together
 *
 * GroupFiguresCommand encapsulates combining multiple figures into a single
 * group. Uses UIFSMAdapter for grouping functionality and stores the group ID
 * and released figure IDs for undo. Supports JSON serialization.
 */
class GroupFiguresCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to group figures
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param figureIds The IDs of the figures to group
   */
  GroupFiguresCommand(UIFSMAdapter &fsmAdapter,
                      const std::vector<uint32_t> &figureIds);

  /**
   * @brief Destructor
   */
  ~GroupFiguresCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter
  UIFSMAdapter &fsmAdapter_;

  /// The IDs of the figures to group
  std::vector<uint32_t> figureIds_;

  /// The ID of the created group (assigned during execute)
  uint32_t groupId_;

  /// The IDs of figures released from their previous groups (for undo)
  std::vector<uint32_t> releasedFigureIds_;

  /// Tracks whether the command has been executed
  bool executed_;
};

} // namespace view
