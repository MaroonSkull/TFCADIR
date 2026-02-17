/**
 * @file SVGImporter.cpp
 * @brief SVG file format importer implementation
 */

#include "SVGImporter.hpp"

#include "View/ImportExport/FileFormats.hpp"

#include <Model/FlatFigure.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace view {
namespace import_export {

SVGImporter::SVGImporter() = default;

ImportResult SVGImporter::import(const std::string &filePath) {
  ImportResult result;

  // Open file
  std::ifstream file(filePath);
  if (!file.is_open()) {
    result.success = false;
    result.errorMessage = "Cannot open file: " + filePath;
    return result;
  }

  // Read entire file content
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  // Parse SVG content
  if (!parseSVG(content, result)) {
    result.success = false;
    if (result.errorMessage.empty()) {
      result.errorMessage = "Failed to parse SVG content";
    }
    return result;
  }

  result.success = true;
  return result;
}

bool SVGImporter::canImport(const std::string &filePath) const {
  // Check file extension
  if (detectFormatFromPath(filePath) != FileFormat::SVG) {
    return false;
  }

  // Try to open and check for SVG signature
  std::ifstream file(filePath);
  if (!file.is_open()) {
    return false;
  }

  std::string line;
  if (std::getline(file, line)) {
    // Check for XML declaration or SVG tag
    if (line.find("<?xml") != std::string::npos ||
        line.find("<svg") != std::string::npos) {
      file.close();
      return true;
    }
  }

  // Check more lines for SVG tag
  while (std::getline(file, line)) {
    if (line.find("<svg") != std::string::npos) {
      file.close();
      return true;
    }
  }

  file.close();
  return false;
}

bool SVGImporter::parseSVG(const std::string &content, ImportResult &result) {
  // Simple regex-based parsing for basic SVG elements
  // Note: A production implementation would use a proper XML parser

  // Parse circles
  std::regex circleRegex(
      R"(<circle[^>]*\s+cx\s*=\s*["']([^"']+)["'][^>]*\s+cy\s*=\s*["']([^"']+)["'][^>]*\s+r\s*=\s*["']([^"']+)["'][^>]*/>|<circle[^>]*\s+r\s*=\s*["']([^"']+)["'][^>]*\s+cx\s*=\s*["']([^"']+)["'][^>]*\s+cy\s*=\s*["']([^"']+)["'][^>]*/>)",
      std::regex::icase);

  std::smatch match;
  std::string::const_iterator searchStart = content.cbegin();

  while (std::regex_search(searchStart, content.cend(), match, circleRegex)) {
    parseCircle(match.str(), result);
    searchStart = match.suffix().first;
  }

  // Parse rectangles
  std::regex rectRegex(R"(<rect[^>]*>)", std::regex::icase);
  searchStart = content.cbegin();

  while (std::regex_search(searchStart, content.cend(), match, rectRegex)) {
    parseRect(match.str(), result);
    searchStart = match.suffix().first;
  }

  // Parse lines
  std::regex lineRegex(R"(<line[^>]*>)", std::regex::icase);
  searchStart = content.cbegin();

  while (std::regex_search(searchStart, content.cend(), match, lineRegex)) {
    parseLine(match.str(), result);
    searchStart = match.suffix().first;
  }

  return true;
}

void SVGImporter::parseCircle(const std::string &element,
                              ImportResult &result) {
  float cx = getAttribute(element, "cx", 0.0f);
  float cy = getAttribute(element, "cy", 0.0f);
  float r = getAttribute(element, "r", 0.0f);

  if (r <= 0.0f) {
    return; // Invalid circle
  }

  // Create Circle figure using the Figure<Circle> template
  auto circle = std::make_shared<model::Figure<model::Circle>>(cx, cy);
  circle->center.x = cx;
  circle->center.y = cy;
  circle->radius = r;

  result.figures.push_back(circle);
}

void SVGImporter::parseRect(const std::string &element, ImportResult &result) {
  float x = getAttribute(element, "x", 0.0f);
  float y = getAttribute(element, "y", 0.0f);
  float width = getAttribute(element, "width", 0.0f);
  float height = getAttribute(element, "height", 0.0f);

  if (width <= 0.0f || height <= 0.0f) {
    return; // Invalid rectangle
  }

  // Create Quad figure from rectangle
  auto quad = std::make_shared<model::Figure<model::Quad>>(x, y);
  quad->first.x = x;
  quad->first.y = y;
  quad->second.x = x + width;
  quad->second.y = y;
  quad->third.x = x + width;
  quad->third.y = y + height;
  quad->fourth.x = x;
  quad->fourth.y = y + height;

  result.figures.push_back(quad);
}

void SVGImporter::parseLine(const std::string &element, ImportResult &result) {
  float x1 = getAttribute(element, "x1", 0.0f);
  float y1 = getAttribute(element, "y1", 0.0f);
  float x2 = getAttribute(element, "x2", 0.0f);
  float y2 = getAttribute(element, "y2", 0.0f);

  // Create a thin quad to represent the line
  // For simplicity, we'll create a line as a very thin quad
  auto line = std::make_shared<model::Figure<model::Quad>>(x1, y1);

  // Calculate perpendicular direction for line width
  glm::vec2 dir(x2 - x1, y2 - y1);
  float length = glm::length(dir);
  if (length < 0.001f) {
    return; // Degenerate line
  }

  dir = glm::normalize(dir);
  glm::vec2 perp(-dir.y, dir.x);
  float lineWidth = 1.0f; // Default line width

  glm::vec2 p1(x1, y1);
  glm::vec2 p2(x2, y2);

  glm::vec2 v1 = p1 + perp * lineWidth * 0.5f;
  glm::vec2 v2 = p1 - perp * lineWidth * 0.5f;
  glm::vec2 v3 = p2 - perp * lineWidth * 0.5f;
  glm::vec2 v4 = p2 + perp * lineWidth * 0.5f;

  line->first.x = v1.x;
  line->first.y = v1.y;
  line->second.x = v2.x;
  line->second.y = v2.y;
  line->third.x = v3.x;
  line->third.y = v3.y;
  line->fourth.x = v4.x;
  line->fourth.y = v4.y;

  result.figures.push_back(line);
}

float SVGImporter::getAttribute(const std::string &element,
                                const std::string &attr, float defaultValue) {
  // Build regex pattern for attribute
  std::string pattern = attr + R"(\s*=\s*["']([^"']+)["'])";
  std::regex attrRegex(pattern, std::regex::icase);
  std::smatch match;

  if (std::regex_search(element, match, attrRegex)) {
    try {
      return std::stof(match[1].str());
    } catch (const std::exception &) {
      return defaultValue;
    }
  }

  return defaultValue;
}

std::string SVGImporter::getStringAttribute(const std::string &element,
                                            const std::string &attr) {
  std::string pattern = attr + R"(\s*=\s*["']([^"']+)["'])";
  std::regex attrRegex(pattern, std::regex::icase);
  std::smatch match;

  if (std::regex_search(element, match, attrRegex)) {
    return match[1].str();
  }

  return "";
}

} // namespace import_export
} // namespace view
