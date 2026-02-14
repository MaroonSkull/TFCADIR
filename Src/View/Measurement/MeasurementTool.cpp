/**
 * @file MeasurementTool.cpp
 * @brief Implementation of measurement tools for Phase 12
 *
 * This file implements all measurement tool classes including distance,
 * area, angle, volume, coordinate, and length measurements.
 */

#include "MeasurementTool.hpp"
#include "View/UIFSMAdapter.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

//------------------------------------------------------------------------------
// MeasurementTool Base Implementation
//------------------------------------------------------------------------------

MeasurementTool::MeasurementTool(Type type, UIFSMAdapter &adapter)
    : type_(type), adapter_(adapter) {}

std::string MeasurementTool::getToolId() const { return typeToString(type_); }

void MeasurementTool::activate() {
  /// Trigger FSM event for tool activation
  adapter_.activateTool(getToolId());
}

void MeasurementTool::deactivate() {
  /// Trigger FSM event for tool deactivation
  adapter_.deactivateTool();
}

bool MeasurementTool::isActive() const {
  /// Check if this tool is currently active
  return adapter_.getActiveTool() == getToolId();
}

std::string MeasurementTool::typeToString(Type type) {
  switch (type) {
  case Type::Distance:
    return "DistanceMeasurement";
  case Type::Area:
    return "AreaMeasurement";
  case Type::Angle:
    return "AngleMeasurement";
  case Type::Coordinate:
    return "CoordinateMeasurement";
  case Type::Length:
    return "LengthMeasurement";
  default:
    return "Unknown";
  }
}

void MeasurementTool::logMeasurement(const std::string &message) const {
  spdlog::info("{}: {}", getToolId(), message);
}

void MeasurementTool::showResult(const std::string &result) const {
  spdlog::info("{}: Result = {}", getToolId(), result);
}

float MeasurementTool::convertToUnit(float pixelValue) const {
  /// Conversion factors from pixels to measurement units
  /// Assuming 96 DPI (standard screen density) for pixel to physical unit
  /// conversion 1 inch = 25.4 mm = 2.54 cm = 0.0254 m At 96 DPI: 1 pixel = 1/96
  /// inch
  constexpr float pixelsPerInch = 96.0f;
  constexpr float inchToMm = 25.4f;
  constexpr float inchToCm = 2.54f;
  constexpr float inchToM = 0.0254f;

  float inches = pixelValue / pixelsPerInch;

  switch (style_.unit) {
  case MeasurementUnit::Millimeters:
    return inches * inchToMm;
  case MeasurementUnit::Centimeters:
    return inches * inchToCm;
  case MeasurementUnit::Meters:
    return inches * inchToM;
  case MeasurementUnit::Inches:
    return inches;
  default:
    return pixelValue;
  }
}

std::string MeasurementTool::getUnitSuffix() const {
  switch (style_.unit) {
  case MeasurementUnit::Millimeters:
    return "mm";
  case MeasurementUnit::Centimeters:
    return "cm";
  case MeasurementUnit::Meters:
    return "m";
  case MeasurementUnit::Inches:
    return "in";
  default:
    return "";
  }
}

std::string MeasurementTool::formatWithUnit(float pixelValue) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << convertToUnit(pixelValue) << " " << getUnitSuffix()
      << style_.suffix;
  return oss.str();
}

//------------------------------------------------------------------------------
// MeasurementToolFactory Implementation
//------------------------------------------------------------------------------

std::unique_ptr<MeasurementTool>
MeasurementToolFactory::createTool(MeasurementTool::Type type,
                                   UIFSMAdapter &adapter) {
  switch (type) {
  case MeasurementTool::Type::Distance:
    return std::make_unique<DistanceMeasurementTool>(adapter);
  case MeasurementTool::Type::Area:
    return std::make_unique<AreaMeasurementTool>(adapter);
  case MeasurementTool::Type::Angle:
    return std::make_unique<AngleMeasurementTool>(adapter);
  case MeasurementTool::Type::Coordinate:
    return std::make_unique<CoordinateMeasurementTool>(adapter);
  case MeasurementTool::Type::Length:
    return std::make_unique<LengthMeasurementTool>(adapter);
  default:
    spdlog::warn("MeasurementToolFactory: Unknown tool type {}",
                 static_cast<int>(type));
    return nullptr;
  }
}

std::vector<MeasurementTool::Type> MeasurementToolFactory::getAvailableTools() {
  return {MeasurementTool::Type::Distance, MeasurementTool::Type::Area,
          MeasurementTool::Type::Angle, MeasurementTool::Type::Coordinate,
          MeasurementTool::Type::Length};
}

std::string
MeasurementToolFactory::getToolDisplayName(MeasurementTool::Type type) {
  switch (type) {
  case MeasurementTool::Type::Distance:
    return "Distance Measurement";
  case MeasurementTool::Type::Area:
    return "Area Measurement";
  case MeasurementTool::Type::Angle:
    return "Angle Measurement";
  case MeasurementTool::Type::Coordinate:
    return "Coordinate Measurement";
  case MeasurementTool::Type::Length:
    return "Length Measurement";
  default:
    return "Unknown Tool";
  }
}

std::string MeasurementToolFactory::getTypeName(MeasurementTool::Type type) {
  switch (type) {
  case MeasurementTool::Type::Distance:
    return "Distance";
  case MeasurementTool::Type::Area:
    return "Area";
  case MeasurementTool::Type::Angle:
    return "Angle";
  case MeasurementTool::Type::Coordinate:
    return "Coordinate";
  case MeasurementTool::Type::Length:
    return "Length";
  default:
    return "Unknown";
  }
}

std::string MeasurementToolFactory::getDescription(MeasurementTool::Type type) {
  switch (type) {
  case MeasurementTool::Type::Distance:
    return "Measure distance between two points";
  case MeasurementTool::Type::Area:
    return "Calculate area of polygon";
  case MeasurementTool::Type::Angle:
    return "Measure angle between three points";
  case MeasurementTool::Type::Coordinate:
    return "Display point coordinates";
  case MeasurementTool::Type::Length:
    return "Measure length along path";
  default:
    return "Unknown measurement type";
  }
}

//------------------------------------------------------------------------------
// DistanceMeasurementTool Implementation
//------------------------------------------------------------------------------

DistanceMeasurementTool::DistanceMeasurementTool(UIFSMAdapter &adapter)
    : MeasurementTool(Type::Distance, adapter), currentMousePos_(0.0f),
      distance_(0.0f) {}

bool DistanceMeasurementTool::onMouseDown(const glm::vec2 &screenPos) {
  /// Add the clicked point
  collectedPoints_.push_back(screenPos);
  logMeasurement("Point " + std::to_string(collectedPoints_.size()) +
                 " added at (" + std::to_string(screenPos.x) + ", " +
                 std::to_string(screenPos.y) + ")");

  /// Check if we have enough points
  if (collectedPoints_.size() >= 2) {
    completeMeasurement();
  }

  return true;
}

bool DistanceMeasurementTool::onMouseMove(const glm::vec2 &screenPos) {
  /// Update current mouse position for preview
  currentMousePos_ = screenPos;
  return true;
}

bool DistanceMeasurementTool::onMouseUp(const glm::vec2 &screenPos) {
  /// Not used for distance measurement
  return false;
}

void DistanceMeasurementTool::renderPreview() const {
  /// Render preview line from first point to current mouse position
  /// The actual rendering is done by the rendering system using the preview
  /// data
  if (!style_.showPreview) {
    return;
  }

  if (collectedPoints_.empty()) {
    return;
  }

  /// Log preview data for debugging - rendering system would use
  /// getStartPoint() and currentMousePos_ to draw a preview line with
  /// style_.lineColor and style_.lineWeight
  spdlog::debug("{}: Preview from ({}, {}) to ({}, {})", getToolId(),
                collectedPoints_[0].x, collectedPoints_[0].y,
                currentMousePos_.x, currentMousePos_.y);
}

void DistanceMeasurementTool::completeMeasurement() {
  if (collectedPoints_.size() >= 2) {
    distance_ = getDistance();
    std::string value = getFormattedValue();
    showResult(value);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(value);
    }

    /// Clear points for next measurement
    collectedPoints_.clear();
  }
}

void DistanceMeasurementTool::cancel() {
  collectedPoints_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  distance_ = 0.0f;
  logMeasurement("Cancelled");
}

size_t DistanceMeasurementTool::getCollectedPointsCount() const {
  return collectedPoints_.size();
}

bool DistanceMeasurementTool::isComplete() const {
  return collectedPoints_.size() >= 2;
}

std::string DistanceMeasurementTool::getFormattedValue() const {
  if (collectedPoints_.size() < 2) {
    return "";
  }
  return formatValue(getDistance());
}

float DistanceMeasurementTool::getDistance() const {
  if (collectedPoints_.size() < 2) {
    return 0.0f;
  }
  return glm::length(collectedPoints_[1] - collectedPoints_[0]);
}

glm::vec2 DistanceMeasurementTool::getStartPoint() const {
  return collectedPoints_.empty() ? glm::vec2(0.0f) : collectedPoints_[0];
}

glm::vec2 DistanceMeasurementTool::getEndPoint() const {
  return collectedPoints_.size() < 2 ? glm::vec2(0.0f) : collectedPoints_[1];
}

std::string DistanceMeasurementTool::formatValue(float value) const {
  return formatWithUnit(value);
}

//------------------------------------------------------------------------------
// AreaMeasurementTool Implementation
//------------------------------------------------------------------------------

AreaMeasurementTool::AreaMeasurementTool(UIFSMAdapter &adapter)
    : MeasurementTool(Type::Area, adapter), currentMousePos_(0.0f),
      area_(0.0f) {}

bool AreaMeasurementTool::onMouseDown(const glm::vec2 &screenPos) {
  /// Add the clicked point
  collectedPoints_.push_back(screenPos);
  logMeasurement("Point " + std::to_string(collectedPoints_.size()) +
                 " added at (" + std::to_string(screenPos.x) + ", " +
                 std::to_string(screenPos.y) + ")");
  return true;
}

bool AreaMeasurementTool::onMouseMove(const glm::vec2 &screenPos) {
  currentMousePos_ = screenPos;
  return true;
}

bool AreaMeasurementTool::onMouseUp(const glm::vec2 &screenPos) {
  /// Double-click or right-click to complete
  return false;
}

void AreaMeasurementTool::renderPreview() const {
  /// Render preview polygon from collected points to current mouse position
  /// The actual rendering is done by the rendering system using the preview
  /// data
  if (!style_.showPreview) {
    return;
  }

  if (collectedPoints_.empty()) {
    return;
  }

  /// Log preview data for debugging - rendering system would use getPoints()
  /// and currentMousePos_ to draw a preview polygon with style_.lineColor and
  /// style_.lineWeight
  std::string pointsStr;
  for (const auto &pt : collectedPoints_) {
    pointsStr +=
        "(" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ") ";
  }
  pointsStr += "-> (" + std::to_string(currentMousePos_.x) + ", " +
               std::to_string(currentMousePos_.y) + ")";
  spdlog::debug("{}: Preview polygon: {}", getToolId(), pointsStr);
}

void AreaMeasurementTool::completeMeasurement() {
  if (collectedPoints_.size() >= 3) {
    area_ = getArea();
    std::string value = getFormattedValue();
    showResult(value);

    if (onComplete_) {
      onComplete_(value);
    }

    collectedPoints_.clear();
  }
}

void AreaMeasurementTool::cancel() {
  collectedPoints_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  area_ = 0.0f;
  logMeasurement("Cancelled");
}

size_t AreaMeasurementTool::getCollectedPointsCount() const {
  return collectedPoints_.size();
}

bool AreaMeasurementTool::isComplete() const {
  return collectedPoints_.size() >= 3;
}

std::string AreaMeasurementTool::getFormattedValue() const {
  if (collectedPoints_.size() < 3) {
    return "";
  }
  return formatValue(getArea());
}

float AreaMeasurementTool::getArea() const {
  if (collectedPoints_.size() < 3) {
    return 0.0f;
  }

  /// Use the Shoelace formula for polygon area
  float area = 0.0f;
  size_t n = collectedPoints_.size();

  for (size_t i = 0; i < n; ++i) {
    size_t j = (i + 1) % n;
    area += collectedPoints_[i].x * collectedPoints_[j].y;
    area -= collectedPoints_[j].x * collectedPoints_[i].y;
  }

  return std::abs(area) / 2.0f;
}

const std::vector<glm::vec2> &AreaMeasurementTool::getPoints() const {
  return collectedPoints_;
}

std::string AreaMeasurementTool::formatValue(float value) const {
  /// Area is in squared units, so we need to square the conversion factor
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix
      << convertToUnit(std::sqrt(value)) * convertToUnit(std::sqrt(value))
      << " " << getUnitSuffix() << "²" << style_.suffix;
  return oss.str();
}

//------------------------------------------------------------------------------
// AngleMeasurementTool Implementation
//------------------------------------------------------------------------------

AngleMeasurementTool::AngleMeasurementTool(UIFSMAdapter &adapter)
    : MeasurementTool(Type::Angle, adapter), currentMousePos_(0.0f),
      angle_(0.0f) {}

bool AngleMeasurementTool::onMouseDown(const glm::vec2 &screenPos) {
  collectedPoints_.push_back(screenPos);
  logMeasurement("Point " + std::to_string(collectedPoints_.size()) +
                 " added at (" + std::to_string(screenPos.x) + ", " +
                 std::to_string(screenPos.y) + ")");

  if (collectedPoints_.size() >= 3) {
    completeMeasurement();
  }

  return true;
}

bool AngleMeasurementTool::onMouseMove(const glm::vec2 &screenPos) {
  currentMousePos_ = screenPos;
  return true;
}

bool AngleMeasurementTool::onMouseUp(const glm::vec2 &screenPos) {
  return false;
}

void AngleMeasurementTool::renderPreview() const {
  /// Render preview angle arc from collected points to current mouse position
  /// The actual rendering is done by the rendering system using the preview
  /// data
  if (!style_.showPreview) {
    return;
  }

  if (collectedPoints_.empty()) {
    return;
  }

  /// Log preview data for debugging - rendering system would use getVertex(),
  /// getFirstPoint(), getSecondPoint() and currentMousePos_ to draw a preview
  /// angle arc with style_.lineColor and style_.lineWeight
  std::string pointsStr;
  for (const auto &pt : collectedPoints_) {
    pointsStr +=
        "(" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ") ";
  }
  pointsStr += "-> mouse (" + std::to_string(currentMousePos_.x) + ", " +
               std::to_string(currentMousePos_.y) + ")";
  spdlog::debug("{}: Preview angle points: {}", getToolId(), pointsStr);
}

void AngleMeasurementTool::completeMeasurement() {
  if (collectedPoints_.size() >= 3) {
    angle_ = getAngle();
    std::string value = getFormattedValue();
    showResult(value);

    if (onComplete_) {
      onComplete_(value);
    }

    collectedPoints_.clear();
  }
}

void AngleMeasurementTool::cancel() {
  collectedPoints_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  angle_ = 0.0f;
  logMeasurement("Cancelled");
}

size_t AngleMeasurementTool::getCollectedPointsCount() const {
  return collectedPoints_.size();
}

bool AngleMeasurementTool::isComplete() const {
  return collectedPoints_.size() >= 3;
}

std::string AngleMeasurementTool::getFormattedValue() const {
  if (collectedPoints_.size() < 3) {
    return "";
  }
  return formatValue(getAngle());
}

float AngleMeasurementTool::getAngle() const {
  if (collectedPoints_.size() < 3) {
    return 0.0f;
  }
  return calculateAngle(collectedPoints_[0], collectedPoints_[1],
                        collectedPoints_[2]);
}

glm::vec2 AngleMeasurementTool::getVertex() const {
  return collectedPoints_.size() < 2 ? glm::vec2(0.0f) : collectedPoints_[1];
}

glm::vec2 AngleMeasurementTool::getFirstPoint() const {
  return collectedPoints_.empty() ? glm::vec2(0.0f) : collectedPoints_[0];
}

glm::vec2 AngleMeasurementTool::getSecondPoint() const {
  return collectedPoints_.size() < 3 ? glm::vec2(0.0f) : collectedPoints_[2];
}

float AngleMeasurementTool::calculateAngle(const glm::vec2 &p1,
                                           const glm::vec2 &vertex,
                                           const glm::vec2 &p2) const {
  /// Calculate vectors from vertex to each point
  glm::vec2 v1 = p1 - vertex;
  glm::vec2 v2 = p2 - vertex;

  /// Calculate angle using atan2 for robustness
  float angle1 = std::atan2(v1.y, v1.x);
  float angle2 = std::atan2(v2.y, v2.x);

  /// Calculate the difference
  float angleDiff = angle2 - angle1;

  /// Normalize to [0, 2*PI] using safe if-based normalization
  /// This prevents potential infinite loops from while statements
  constexpr float twoPi = 2.0f * glm::pi<float>();

  /// Normalize: add/subtract 2*PI once if needed
  if (angleDiff < 0.0f) {
    angleDiff += twoPi;
  }
  if (angleDiff > twoPi) {
    angleDiff -= twoPi;
  }

  /// Safety check: use fmod for final normalization if still out of range
  if (angleDiff < 0.0f || angleDiff > twoPi) {
    angleDiff = std::fmod(angleDiff, twoPi);
    if (angleDiff < 0.0f) {
      angleDiff += twoPi;
    }
  }

  /// Convert to degrees
  return glm::degrees(angleDiff);
}

std::string AngleMeasurementTool::formatValue(float value) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << value << "°" << style_.suffix;
  return oss.str();
}

//------------------------------------------------------------------------------
// CoordinateMeasurementTool Implementation
//------------------------------------------------------------------------------

CoordinateMeasurementTool::CoordinateMeasurementTool(UIFSMAdapter &adapter)
    : MeasurementTool(Type::Coordinate, adapter), currentMousePos_(0.0f) {}

bool CoordinateMeasurementTool::onMouseDown(const glm::vec2 &screenPos) {
  collectedPoints_.push_back(screenPos);
  std::string coord = formatCoordinate(screenPos);
  logMeasurement("Coordinate " + std::to_string(collectedPoints_.size()) +
                 ": " + coord);
  showResult(coord);

  if (onComplete_) {
    onComplete_(coord);
  }

  return true;
}

bool CoordinateMeasurementTool::onMouseMove(const glm::vec2 &screenPos) {
  currentMousePos_ = screenPos;
  return true;
}

bool CoordinateMeasurementTool::onMouseUp(const glm::vec2 &screenPos) {
  return false;
}

void CoordinateMeasurementTool::renderPreview() const {
  /// Render preview coordinate at current mouse position
  /// The actual rendering is done by the rendering system using the preview
  /// data
  if (!style_.showPreview) {
    return;
  }

  /// Log preview data for debugging - rendering system would use
  /// currentMousePos_ to draw a preview coordinate marker with style_.color
  spdlog::debug("{}: Preview coordinate at ({}, {})", getToolId(),
                currentMousePos_.x, currentMousePos_.y);
}

void CoordinateMeasurementTool::completeMeasurement() {
  /// Coordinate measurement is complete on each click
}

void CoordinateMeasurementTool::cancel() {
  collectedPoints_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  logMeasurement("Cancelled");
}

size_t CoordinateMeasurementTool::getCollectedPointsCount() const {
  return collectedPoints_.size();
}

bool CoordinateMeasurementTool::isComplete() const {
  /// Coordinate measurement is always "complete" after each point
  return !collectedPoints_.empty();
}

std::string CoordinateMeasurementTool::getFormattedValue() const {
  if (collectedPoints_.empty()) {
    return "";
  }
  return formatCoordinate(collectedPoints_.back());
}

glm::vec2 CoordinateMeasurementTool::getLastCoordinate() const {
  return collectedPoints_.empty() ? glm::vec2(0.0f) : collectedPoints_.back();
}

const std::vector<glm::vec2> &
CoordinateMeasurementTool::getCoordinates() const {
  return collectedPoints_;
}

std::string
CoordinateMeasurementTool::formatCoordinate(const glm::vec2 &coord) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << "X: " << convertToUnit(coord.x) << " " << getUnitSuffix() << ", "
      << "Y: " << convertToUnit(coord.y) << " " << getUnitSuffix();
  return oss.str();
}

//------------------------------------------------------------------------------
// LengthMeasurementTool Implementation
//------------------------------------------------------------------------------

LengthMeasurementTool::LengthMeasurementTool(UIFSMAdapter &adapter)
    : MeasurementTool(Type::Length, adapter), currentMousePos_(0.0f),
      length_(0.0f) {}

bool LengthMeasurementTool::onMouseDown(const glm::vec2 &screenPos) {
  collectedPoints_.push_back(screenPos);
  logMeasurement("Point " + std::to_string(collectedPoints_.size()) +
                 " added at (" + std::to_string(screenPos.x) + ", " +
                 std::to_string(screenPos.y) + ")");
  return true;
}

bool LengthMeasurementTool::onMouseMove(const glm::vec2 &screenPos) {
  currentMousePos_ = screenPos;
  return true;
}

bool LengthMeasurementTool::onMouseUp(const glm::vec2 &screenPos) {
  /// Double-click or right-click to complete
  return false;
}

void LengthMeasurementTool::renderPreview() const {
  /// Render preview path from collected points to current mouse position
  /// The actual rendering is done by the rendering system using the preview
  /// data
  if (!style_.showPreview) {
    return;
  }

  if (collectedPoints_.empty()) {
    return;
  }

  /// Log preview data for debugging - rendering system would use getPoints()
  /// and currentMousePos_ to draw a preview path with style_.lineColor and
  /// style_.lineWeight
  std::string pointsStr;
  for (const auto &pt : collectedPoints_) {
    pointsStr +=
        "(" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ") ";
  }
  pointsStr += "-> mouse (" + std::to_string(currentMousePos_.x) + ", " +
               std::to_string(currentMousePos_.y) + ")";
  spdlog::debug("{}: Preview path: {}", getToolId(), pointsStr);
}

void LengthMeasurementTool::completeMeasurement() {
  if (collectedPoints_.size() >= 2) {
    length_ = getLength();
    std::string value = getFormattedValue();
    showResult(value);

    if (onComplete_) {
      onComplete_(value);
    }

    collectedPoints_.clear();
  }
}

void LengthMeasurementTool::cancel() {
  collectedPoints_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  length_ = 0.0f;
  logMeasurement("Cancelled");
}

size_t LengthMeasurementTool::getCollectedPointsCount() const {
  return collectedPoints_.size();
}

bool LengthMeasurementTool::isComplete() const {
  return collectedPoints_.size() >= 2;
}

std::string LengthMeasurementTool::getFormattedValue() const {
  if (collectedPoints_.size() < 2) {
    return "";
  }
  return formatValue(getLength());
}

float LengthMeasurementTool::getLength() const {
  if (collectedPoints_.size() < 2) {
    return 0.0f;
  }

  /// Sum up distances between consecutive points
  float totalLength = 0.0f;
  for (size_t i = 1; i < collectedPoints_.size(); ++i) {
    totalLength += glm::length(collectedPoints_[i] - collectedPoints_[i - 1]);
  }

  return totalLength;
}

const std::vector<glm::vec2> &LengthMeasurementTool::getPoints() const {
  return collectedPoints_;
}

std::string LengthMeasurementTool::formatValue(float value) const {
  return formatWithUnit(value);
}

} // namespace view
