#pragma once

#include "ICommand.hpp"
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for ungrouping figures
 *
 * UngroupFiguresCommand encapsulates releasing figures from a group.
 * Uses UIFSMAdapter for ungrouping functionality and stores the released
 * figure IDs for undo. Supports JSON serialization.
 */
class UngroupFiguresCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to ungroup figures
   * @param fsmAdapter Reference to the UIFSMAdapter
   * @param groupId The ID of the group to ungroup
   */
  UngroupFiguresCommand(UIFSMAdapter &fsmAdapter, uint32_t groupId);

  /**
   * @brief Destructor
   */
  ~UngroupFiguresCommand() override = default;

  bool execute() override;
  bool undo() override;
  std::string getDescription() const override;
  std::string serialize() const override;
  std::string getType() const override;
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// Reference to the UIFSMAdapter
  UIFSMAdapter &fsmAdapter_;

  /// The ID of the group to ungroup
  uint32_t groupId_;

  /// The IDs of figures released from the group (assigned during execute)
  std::vector<uint32_t> releasedFigureIds_;

  /// Tracks whether the command has been executed
  bool executed_;
};

} // namespace view
