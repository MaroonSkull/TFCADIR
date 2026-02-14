/**
 * @file DXFImporter.cpp
 * @brief Implementation of DXF file format importer
 */

#include "DXFImporter.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <sstream>

namespace view {
namespace import_export {

DXFImporter::DXFImporter() = default;

bool DXFImporter::canImport(const std::string &filePath) const {
  // Check file extension
  FileFormat format = detectFormatFromPath(filePath);
  if (format != FileFormat::DXF) {
    return false;
  }

  // Try to open the file and check for DXF header
  std::ifstream file(filePath);
  if (!file.is_open()) {
    return false;
  }

  // Read first group code
  int groupCode = 0;
  std::string value;
  if (!readGroupCode(file, groupCode, value)) {
    file.close();
    return false;
  }

  // Check for DXF header indicator
  bool isValid = (groupCode == 0 && value == "SECTION");

  file.close();
  return isValid;
}

ImportResult DXFImporter::import(const std::string &filePath) {
  ImportResult result;

  // Open the file
  std::ifstream file(filePath);
  if (!file.is_open()) {
    result.success = false;
    result.errorMessage = "Cannot open file: " + filePath;
    spdlog::error("DXFImporter: {}", result.errorMessage);
    return result;
  }

  // Parse DXF entities
  std::vector<DXFEntity> entities;
  if (!parseDXF(file, entities)) {
    result.success = false;
    result.errorMessage = "Failed to parse DXF file";
    spdlog::error("DXFImporter: {}", result.errorMessage);
    file.close();
    return result;
  }

  file.close();

  // Convert entities to figures
  spdlog::info("DXFImporter: Found {} entities", entities.size());

  for (const auto &entity : entities) {
    auto figure = convertEntity(entity);
    if (figure) {
      result.figures.push_back(figure);
    }
  }

  result.success = true;
  spdlog::info("DXFImporter: Successfully imported {} figures",
               result.figures.size());

  return result;
}

bool DXFImporter::parseDXF(std::ifstream &file,
                           std::vector<DXFEntity> &entities) {
  int groupCode = 0;
  std::string value;
  DXFEntity currentEntity;
  bool inEntitiesSection = false;
  bool inEntity = false;

  while (readGroupCode(file, groupCode, value)) {
    // Trim whitespace from value
    value.erase(0, value.find_first_not_of(" \t\n\r"));
    value.erase(value.find_last_not_of(" \t\n\r") + 1);

    if (groupCode == 0) {
      if (value == "SECTION") {
        // Start of a section
        continue;
      }
      if (value == "ENDSEC") {
        // End of section
        if (inEntity && currentEntity.type != DXFEntityType::Unknown) {
          entities.push_back(currentEntity);
        }
        inEntitiesSection = false;
        inEntity = false;
        currentEntity = DXFEntity();
        continue;
      }
      if (value == "EOF") {
        // End of file
        break;
      }

      // Check for section type
      if (!inEntitiesSection) {
        continue;
      }

      // Save previous entity if any
      if (inEntity && currentEntity.type != DXFEntityType::Unknown) {
        entities.push_back(currentEntity);
      }

      // Start new entity
      inEntity = true;
      currentEntity = DXFEntity();

      if (value == "LINE") {
        currentEntity.type = DXFEntityType::Line;
      } else if (value == "CIRCLE") {
        currentEntity.type = DXFEntityType::Circle;
      } else if (value == "ARC") {
        currentEntity.type = DXFEntityType::Arc;
      } else if (value == "POLYLINE") {
        currentEntity.type = DXFEntityType::Polyline;
      } else if (value == "LWPOLYLINE") {
        currentEntity.type = DXFEntityType::LWPolyline;
      } else {
        currentEntity.type = DXFEntityType::Unknown;
        inEntity = false;
      }
    } else if (groupCode == 2) {
      // Section name
      if (value == "ENTITIES") {
        inEntitiesSection = true;
      }
    } else if (inEntity) {
      // Parse entity data based on group code
      switch (groupCode) {
      case 10: // X coordinate of first point
        currentEntity.x1 = std::stof(value);
        break;
      case 20: // Y coordinate of first point
        currentEntity.y1 = std::stof(value);
        break;
      case 30: // Z coordinate of first point
        currentEntity.z1 = std::stof(value);
        break;
      case 11: // X coordinate of second point (for LINE)
        currentEntity.x2 = std::stof(value);
        break;
      case 21: // Y coordinate of second point
        currentEntity.y2 = std::stof(value);
        break;
      case 31: // Z coordinate of second point
        currentEntity.z2 = std::stof(value);
        break;
      case 40: // Radius
        currentEntity.radius = std::stof(value);
        break;
      case 62: // Layer/color number
        currentEntity.layerIndex = std::stoi(value);
        break;
      default:
        // Handle polyline vertices
        if (currentEntity.type == DXFEntityType::LWPolyline ||
            currentEntity.type == DXFEntityType::Polyline) {
          if (groupCode == 10) {
            // Start a new vertex
            if (!currentEntity.vertices.empty() || currentEntity.x1 != 0.0f) {
              currentEntity.vertices.push_back(
                  {currentEntity.x1, currentEntity.y1});
            }
            currentEntity.x1 = std::stof(value);
          } else if (groupCode == 20) {
            currentEntity.y1 = std::stof(value);
          }
        }
        break;
      }
    }
  }

  // Add last polyline vertex if needed
  if (inEntity && (currentEntity.type == DXFEntityType::LWPolyline ||
                   currentEntity.type == DXFEntityType::Polyline)) {
    currentEntity.vertices.push_back({currentEntity.x1, currentEntity.y1});
  }

  return true;
}

bool DXFImporter::readGroupCode(std::ifstream &file, int &groupCode,
                                std::string &value) {
  std::string line;

  // Read group code
  if (!std::getline(file, line)) {
    return false;
  }

  // Trim whitespace
  line.erase(0, line.find_first_not_of(" \t\n\r"));
  line.erase(line.find_last_not_of(" \t\n\r") + 1);

  // Parse group code
  try {
    groupCode = std::stoi(line);
  } catch (...) {
    return false;
  }

  // Read value
  if (!std::getline(file, value)) {
    return false;
  }

  return true;
}

std::shared_ptr<model::IFigure>
DXFImporter::convertEntity(const DXFEntity &entity) {
  switch (entity.type) {
  case DXFEntityType::Line: {
    // Create a line as a thin quad (simplified as triangle for now)
    auto figure = std::make_shared<model::Figure<model::Triangle>>(
        (entity.x1 + entity.x2) / 2.0f, (entity.y1 + entity.y2) / 2.0f,
        (entity.z1 + entity.z2) / 2.0f);
    figure->first = {entity.x1, entity.y1, entity.z1};
    figure->second = {entity.x2, entity.y2, entity.z2};
    // Third point slightly offset to create a thin triangle
    float dx = entity.x2 - entity.x1;
    float dy = entity.y2 - entity.y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.001f) {
      // Perpendicular offset
      float px = -dy / len * 0.01f;
      float py = dx / len * 0.01f;
      figure->third = {entity.x1 + px, entity.y1 + py, entity.z1};
    } else {
      figure->third = figure->first;
    }
    figure->setName("DXF Line");
    return figure;
  }

  case DXFEntityType::Circle: {
    auto figure = std::make_shared<model::Figure<model::Circle>>(
        entity.centerX, entity.centerY, 0.0f);
    figure->center = {entity.centerX, entity.centerY, 0.0f};
    figure->radius = entity.radius;
    figure->setName("DXF Circle");
    return figure;
  }

  case DXFEntityType::Arc: {
    // Convert arc to circle (simplified)
    auto figure = std::make_shared<model::Figure<model::Circle>>(
        entity.centerX, entity.centerY, 0.0f);
    figure->center = {entity.centerX, entity.centerY, 0.0f};
    figure->radius = entity.radius;
    figure->setName("DXF Arc");
    return figure;
  }

  case DXFEntityType::LWPolyline:
  case DXFEntityType::Polyline: {
    // Convert polyline vertices to triangles (simplified)
    if (entity.vertices.size() < 2) {
      return nullptr;
    }

    // For now, just create a triangle from first three vertices
    // A full implementation would create multiple line segments
    auto figure = std::make_shared<model::Figure<model::Triangle>>(
        entity.vertices[0].first, entity.vertices[0].second, 0.0f);

    if (entity.vertices.size() >= 3) {
      figure->first = {entity.vertices[0].first, entity.vertices[0].second,
                       0.0f};
      figure->second = {entity.vertices[1].first, entity.vertices[1].second,
                        0.0f};
      figure->third = {entity.vertices[2].first, entity.vertices[2].second,
                       0.0f};
    } else {
      // Only 2 vertices - create degenerate triangle
      figure->first = {entity.vertices[0].first, entity.vertices[0].second,
                       0.0f};
      figure->second = {entity.vertices[1].first, entity.vertices[1].second,
                        0.0f};
      figure->third = figure->first;
    }
    figure->setName("DXF Polyline");
    return figure;
  }

  case DXFEntityType::Unknown:
  default:
    spdlog::debug("DXFImporter: Skipping unknown entity type");
    return nullptr;
  }
}

bool DXFImporter::skipToSection(std::ifstream &file,
                                const std::string &sectionName) {
  int groupCode = 0;
  std::string value;

  while (readGroupCode(file, groupCode, value)) {
    if (groupCode == 0 && value == "SECTION") {
      // Read section name
      if (readGroupCode(file, groupCode, value)) {
        if (groupCode == 2 && value == sectionName) {
          return true;
        }
      }
    }
  }

  return false;
}

} // namespace import_export
} // namespace view
