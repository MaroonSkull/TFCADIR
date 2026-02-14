#pragma once

#include "ICommand.hpp"
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace view {

/**
 * @brief Factory for creating commands from JSON data
 *
 * CommandFactory deserializes commands from JSON format, supporting
 * all command types in the system. Used for command persistence
 * and session recovery.
 */
class CommandFactory {
public:
  /**
   * @brief Register a command type with its deserializer
   * @param type Command type identifier (e.g., "CreateFigure", "MoveFigure")
   * @param factory Function that creates a command from JSON
   *
   * Registers a command type so it can be deserialized from JSON.
   * Each command type must register itself with the factory.
   */
  void registerCommand(
      const std::string &type,
      std::function<std::unique_ptr<Commands::ICommand>(const nlohmann::json &)>
          factory);

  /**
   * @brief Deserialize a command from JSON string
   * @param json JSON string representation of the command
   * @return Unique pointer to the deserialized command, or nullptr on failure
   *
   * Parses the JSON string and creates the appropriate command object
   * based on the "type" field. Returns nullptr if type is not registered
   * or deserialization fails.
   */
  std::unique_ptr<Commands::ICommand>
  deserialize(const std::string &json) const;

  /**
   * @brief Check if a command type is registered
   * @param type Command type identifier
   * @return true if the type is registered, false otherwise
   */
  bool isRegistered(const std::string &type) const;

private:
  /// Map of command type identifiers to factory functions
  std::unordered_map<std::string,
                     std::function<std::unique_ptr<Commands::ICommand>(
                         const nlohmann::json &)>>
      factories_;

}; // class CommandFactory

} // namespace view
