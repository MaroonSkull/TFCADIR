#pragma once

#include "ICommand.hpp"
#include <Model/FlatFigure.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Command for creating new figures in the model
 *
 * CreateFigureCommand encapsulates the creation of any figure type
 * (Triangle, Quad, Circle, Ngon, CurveBezier3, CurveBezier4) with
 * complete JSON serialization for persistence and session recovery.
 * Supports the isScribed enum for inscribed/circumscribed figures.
 */
class CreateFigureCommand : public ICommand {
public:
  /**
   * @brief Constructs a command to create a figure
   * @param model Reference to the FlatFigures model
   * @param figure Shared pointer to the figure to create
   */
  CreateFigureCommand(model::FlatFigures &model,
                      std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Destructor
   */
  ~CreateFigureCommand() override = default;

  /**
   * @brief Executes the command by adding the figure to the model
   * @return true if the figure was successfully added, false otherwise
   */
  bool execute() override;

  /**
   * @brief Undoes the command by removing the figure from the model
   * @return true if the figure was successfully removed, false otherwise
   */
  bool undo() override;

  /**
   * @brief Gets a human-readable description of the command
   * @return Description string for UI display
   */
  std::string getDescription() const override;

  /**
   * @brief Serializes the command to JSON format
   * @return JSON string representation of the command
   */
  std::string serialize() const override;

  /**
   * @brief Gets the command type identifier
   * @return "CreateFigure" string for factory deserialization
   */
  std::string getType() const override;

  /**
   * @brief Gets the figure IDs affected by this command
   * @return Vector containing the created figure's ID
   */
  std::vector<uint32_t> getAffectedFigures() const override;

  /**
   * @brief Gets the figure ID assigned after execution
   * @return The figure's ID in the model, or 0 if not yet executed
   */
  uint32_t getFigureId() const { return figureId_; }

private:
  /// Reference to the FlatFigures model
  model::FlatFigures &model_;

  /// The figure to create (stored as shared_ptr to maintain polymorphism)
  std::shared_ptr<model::IFigure> figure_;

  /// The figure's ID in the model (assigned during execute)
  uint32_t figureId_;

  /// Tracks whether the command has been executed
  bool executed_;

  /**
   * @brief Serializes a Triangle figure to JSON
   * @param figure The triangle figure to serialize
   * @return JSON object containing triangle data
   */
  nlohmann::json
  serializeTriangle(const model::Figure<model::Triangle> *figure) const;

  /**
   * @brief Serializes a Quad figure to JSON
   * @param figure The quad figure to serialize
   * @return JSON object containing quad data
   */
  nlohmann::json serializeQuad(const model::Figure<model::Quad> *figure) const;

  /**
   * @brief Serializes a Circle figure to JSON
   * @param figure The circle figure to serialize
   * @return JSON object containing circle data
   */
  nlohmann::json
  serializeCircle(const model::Figure<model::Circle> *figure) const;

  /**
   * @brief Serializes an Ngon figure to JSON
   * @param figure The ngon figure to serialize
   * @return JSON object containing ngon data
   */
  nlohmann::json serializeNgon(const model::Figure<model::Ngon> *figure) const;

  /**
   * @brief Serializes a CurveBezier3 figure to JSON
   * @param figure The bezier3 curve to serialize
   * @return JSON object containing bezier3 data
   */
  nlohmann::json
  serializeBezier3(const model::Figure<model::CurveBezier3> *figure) const;

  /**
   * @brief Serializes a CurveBezier4 figure to JSON
   * @param figure The bezier4 curve to serialize
   * @return JSON object containing bezier4 data
   */
  nlohmann::json
  serializeBezier4(const model::Figure<model::CurveBezier4> *figure) const;

  /**
   * @brief Helper to convert a Point to JSON object
   * @param point The point to convert
   * @return JSON object with x, y, z fields
   */
  nlohmann::json pointToJson(const model::Point &point) const;

  /**
   * @brief Helper to convert a glm::vec3 to JSON object
   * @param vec The vector to convert
   * @return JSON object with x, y, z fields
   */
  nlohmann::json vec3ToJson(const glm::vec3 &vec) const;
};

} // namespace view
