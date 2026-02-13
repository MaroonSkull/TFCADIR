#include "CreateFigureCommand.hpp"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

CreateFigureCommand::CreateFigureCommand(model::FlatFigures &model,
                                         std::shared_ptr<model::IFigure> figure)
    : model_(model), figure_(std::move(figure)), figureId_(0),
      executed_(false) {}

void CreateFigureCommand::execute() {
  if (executed_) {
    spdlog::warn("CreateFigureCommand: Already executed");
    return;
  }

  if (!figure_) {
    spdlog::error("CreateFigureCommand: Figure is null");
    return;
  }

  try {
    // Store the vector index before adding (it will be the last index after
    // add)
    size_t figureIndex = model_.getFigureCount();
    model_.addFigurePtr(figure_);
    // Store the vector index for removal, not the unique ID
    // NOTE: figureId_ is the vector index in FlatFigures, not the unique ID
    figureId_ = static_cast<uint32_t>(figureIndex);
    executed_ = true;
    spdlog::info("CreateFigureCommand: Created figure at vector index {} "
                 "(unique ID: {})",
                 figureId_, figure_->getId());
    return;
  } catch (const std::exception &e) {
    spdlog::error("CreateFigureCommand: Failed to add figure: {}", e.what());
    return;
  }
}

void CreateFigureCommand::undo() {
  if (!executed_) {
    spdlog::warn("CreateFigureCommand: Not executed, cannot undo");
    return;
  }

  try {
    // Remove the figure using the vector index
    bool removed = model_.removeFigure(figureId_);
    if (removed) {
      executed_ = false;
      spdlog::info("CreateFigureCommand: Removed figure at vector index {}",
                   figureId_);
      return;
    } else {
      spdlog::error(
          "CreateFigureCommand: Failed to remove figure at vector index {}",
          figureId_);
      return;
    }
  } catch (const std::exception &e) {
    spdlog::error("CreateFigureCommand: Failed to undo: {}", e.what());
    return;
  }
}

std::string CreateFigureCommand::getDescription() const {
  std::ostringstream oss;
  if (figure_) {
    oss << "Create " << figure_->getName();
  } else {
    oss << "Create Figure";
  }
  return oss.str();
}

std::string CreateFigureCommand::getType() const { return "CreateFigure"; }

std::vector<uint32_t> CreateFigureCommand::getAffectedFigures() const {
  if (executed_ && figureId_ > 0) {
    return {figureId_};
  }
  return {};
}

std::string CreateFigureCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "CreateFigure";
  j["executed"] = executed_;
  j["figureId"] = figureId_;

  if (figure_) {
    j["name"] = figure_->getName();
    glm::vec3 position = figure_->getPosition();
    j["position"] = vec3ToJson(position);

    // Serialize figure-specific data based on type
    if (auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
            figure_)) {
      j["figureType"] = "triangle";
      j["figureData"] = serializeTriangle(tri.get());
    } else if (auto quad =
                   std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                       figure_)) {
      j["figureType"] = "quad";
      j["figureData"] = serializeQuad(quad.get());
    } else if (auto circle =
                   std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                       figure_)) {
      j["figureType"] = "circle";
      j["figureData"] = serializeCircle(circle.get());
    } else if (auto ngon =
                   std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                       figure_)) {
      j["figureType"] = "ngon";
      j["figureData"] = serializeNgon(ngon.get());
    } else if (auto bezier3 = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier3>>(figure_)) {
      j["figureType"] = "bezier3";
      j["figureData"] = serializeBezier3(bezier3.get());
    } else if (auto bezier4 = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier4>>(figure_)) {
      j["figureType"] = "bezier4";
      j["figureData"] = serializeBezier4(bezier4.get());
    }
  }

  return j.dump();
}

nlohmann::json CreateFigureCommand::serializeTriangle(
    const model::Figure<model::Triangle> *figure) const {
  nlohmann::json j;
  j["first"] = pointToJson(figure->first);
  j["second"] = pointToJson(figure->second);
  j["third"] = pointToJson(figure->third);
  return j;
}

nlohmann::json CreateFigureCommand::serializeQuad(
    const model::Figure<model::Quad> *figure) const {
  nlohmann::json j;
  j["first"] = pointToJson(figure->first);
  j["second"] = pointToJson(figure->second);
  j["third"] = pointToJson(figure->third);
  j["fourth"] = pointToJson(figure->fourth);
  return j;
}

nlohmann::json CreateFigureCommand::serializeCircle(
    const model::Figure<model::Circle> *figure) const {
  nlohmann::json j;
  j["center"] = pointToJson(figure->center);
  j["radius"] = figure->radius;
  return j;
}

nlohmann::json CreateFigureCommand::serializeNgon(
    const model::Figure<model::Ngon> *figure) const {
  nlohmann::json j;
  j["center"] = pointToJson(figure->center);
  j["first"] = pointToJson(figure->first);
  j["n"] = figure->n;
  j["radius"] = figure->radius;
  j["scribed"] = static_cast<int>(figure->scribed_);
  return j;
}

nlohmann::json CreateFigureCommand::serializeBezier3(
    const model::Figure<model::CurveBezier3> *figure) const {
  nlohmann::json j;
  j["start"] = pointToJson(figure->start);
  j["end"] = pointToJson(figure->end);
  j["first"] = pointToJson(figure->first);
  return j;
}

nlohmann::json CreateFigureCommand::serializeBezier4(
    const model::Figure<model::CurveBezier4> *figure) const {
  nlohmann::json j;
  j["start"] = pointToJson(figure->start);
  j["end"] = pointToJson(figure->end);
  j["first"] = pointToJson(figure->first);
  j["second"] = pointToJson(figure->second);
  return j;
}

nlohmann::json
CreateFigureCommand::pointToJson(const model::Point &point) const {
  nlohmann::json j;
  j["x"] = point.x;
  j["y"] = point.y;
  j["z"] = point.z;
  return j;
}

nlohmann::json CreateFigureCommand::vec3ToJson(const glm::vec3 &vec) const {
  nlohmann::json j;
  j["x"] = vec.x;
  j["y"] = vec.y;
  j["z"] = vec.z;
  return j;
}

} // namespace view
