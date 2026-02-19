/**
 * @file SVGExporter.cpp
 * @brief SVG file format exporter implementation
 */

#include "SVGExporter.hpp"

// Must be defined before including <cmath> for M_PI to be available on MSVC
#define _USE_MATH_DEFINES
#include <cmath>

#include <Model/FlatFigure.hpp>

#include <fstream>
#include <sstream>

namespace view {
namespace import_export {

SVGExporter::SVGExporter() = default;

ExportResult SVGExporter::exportToFile(
    const std::vector<std::shared_ptr<model::IFigure>> &figures,
    const std::string &filePath) {

  ExportResult result;

  if (figures.empty()) {
    result.success = false;
    result.errorMessage = "No figures to export";
    return result;
  }

  // Generate SVG content
  std::string svgContent = generateSVG(figures);

  // Write to file
  std::ofstream file(filePath);
  if (!file.is_open()) {
    result.success = false;
    result.errorMessage = "Cannot open file for writing: " + filePath;
    return result;
  }

  file << svgContent;
  file.close();

  result.success = true;
  result.figuresExported = figures.size();
  return result;
}

bool SVGExporter::supportsFigure(
    const std::shared_ptr<model::IFigure> &figure) const {
  // SVG supports all figure types
  return figure != nullptr;
}

std::string SVGExporter::generateSVG(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {

  std::stringstream ss;

  // Calculate bounds for viewBox
  auto bounds = calculateBounds(figures);
  float minX = bounds.first.x - 10.0f;
  float minY = bounds.first.y - 10.0f;
  float width = bounds.second.x - bounds.first.x + 20.0f;
  float height = bounds.second.y - bounds.first.y + 20.0f;

  // SVG header
  ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
  ss << "viewBox=\"" << minX << " " << minY << " " << width << " " << height
     << "\">\n";

  // Add a group for all figures
  ss << "  <g id=\"tfcadir-figures\">\n";

  // Export each figure
  for (const auto &figure : figures) {
    if (!figure) {
      continue;
    }

    // Try to cast to each figure type
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      ss << triangleToSVG(figure);
    } else if (auto quad =
                   std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                       figure)) {
      ss << quadToSVG(figure);
    } else if (auto circle =
                   std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                       figure)) {
      ss << circleToSVG(figure);
    } else if (auto ngon =
                   std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                       figure)) {
      ss << ngonToSVG(figure);
    } else if (auto curve3 = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier3>>(figure)) {
      ss << curveBezier3ToSVG(figure);
    } else if (auto curve4 = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier4>>(figure)) {
      ss << curveBezier4ToSVG(figure);
    }
  }

  ss << "  </g>\n";
  ss << "</svg>\n";

  return ss.str();
}

std::string
SVGExporter::triangleToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure);
  if (!tri) {
    return "";
  }

  std::stringstream ss;
  ss << "    <polygon points=\"";
  ss << tri->first.x << "," << tri->first.y << " ";
  ss << tri->second.x << "," << tri->second.y << " ";
  ss << tri->third.x << "," << tri->third.y << "\" ";

  // Add appearance
  auto appearance = tri->getAppearance();
  ss << "fill=\"" << colorToSVG(appearance.fillColor) << "\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string
SVGExporter::quadToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure);
  if (!quad) {
    return "";
  }

  std::stringstream ss;
  ss << "    <polygon points=\"";
  ss << quad->first.x << "," << quad->first.y << " ";
  ss << quad->second.x << "," << quad->second.y << " ";
  ss << quad->third.x << "," << quad->third.y << " ";
  ss << quad->fourth.x << "," << quad->fourth.y << "\" ";

  // Add appearance
  auto appearance = quad->getAppearance();
  ss << "fill=\"" << colorToSVG(appearance.fillColor) << "\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string
SVGExporter::circleToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto circle = std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure);
  if (!circle) {
    return "";
  }

  std::stringstream ss;
  ss << "    <circle cx=\"" << circle->center.x << "\" ";
  ss << "cy=\"" << circle->center.y << "\" ";
  ss << "r=\"" << circle->radius << "\" ";

  // Add appearance
  auto appearance = circle->getAppearance();
  ss << "fill=\"" << colorToSVG(appearance.fillColor) << "\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string
SVGExporter::ngonToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure);
  if (!ngon) {
    return "";
  }

  std::stringstream ss;
  ss << "    <polygon points=\"";

  // Calculate vertices of the n-gon
  int n = static_cast<int>(ngon->n);
  if (n < 3) {
    n = 3; // Minimum is triangle
  }

  // Calculate angle offset from first point
  float firstAngle = std::atan2(ngon->first.y - ngon->center.y,
                                ngon->first.x - ngon->center.x);

  for (int i = 0; i < n; ++i) {
    float angle = firstAngle + (2.0f * M_PI * i) / n;
    float x = ngon->center.x + ngon->radius * std::cos(angle);
    float y = ngon->center.y + ngon->radius * std::sin(angle);
    ss << x << "," << y;
    if (i < n - 1) {
      ss << " ";
    }
  }
  ss << "\" ";

  // Add appearance
  auto appearance = ngon->getAppearance();
  ss << "fill=\"" << colorToSVG(appearance.fillColor) << "\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string
SVGExporter::curveBezier3ToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto curve =
      std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure);
  if (!curve) {
    return "";
  }

  std::stringstream ss;
  ss << "    <path d=\"M " << curve->start.x << "," << curve->start.y << " ";
  ss << "Q " << curve->first.x << "," << curve->first.y << " ";
  ss << curve->end.x << "," << curve->end.y << "\" ";

  // Add appearance
  auto appearance = curve->getAppearance();
  ss << "fill=\"none\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string
SVGExporter::curveBezier4ToSVG(const std::shared_ptr<model::IFigure> &figure) {
  auto curve =
      std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure);
  if (!curve) {
    return "";
  }

  std::stringstream ss;
  ss << "    <path d=\"M " << curve->start.x << "," << curve->start.y << " ";
  ss << "C " << curve->first.x << "," << curve->first.y << " ";
  ss << curve->second.x << "," << curve->second.y << " ";
  ss << curve->end.x << "," << curve->end.y << "\" ";

  // Add appearance
  auto appearance = curve->getAppearance();
  ss << "fill=\"none\" ";
  ss << "stroke=\"" << colorToSVG(appearance.strokeColor) << "\" ";
  ss << "stroke-width=\"" << appearance.lineWidth << "\"/>\n";

  return ss.str();
}

std::string SVGExporter::colorToSVG(const glm::vec3 &color) {
  std::stringstream ss;
  ss << "rgb(" << static_cast<int>(color.r * 255.0f) << ",";
  ss << static_cast<int>(color.g * 255.0f) << ",";
  ss << static_cast<int>(color.b * 255.0f) << ")";
  return ss.str();
}

std::pair<glm::vec3, glm::vec3> SVGExporter::calculateBounds(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {

  glm::vec3 minBounds(std::numeric_limits<float>::max());
  glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

  for (const auto &figure : figures) {
    if (!figure) {
      continue;
    }

    auto bounds = figure->getBounds();
    minBounds = glm::min(minBounds, bounds.first);
    maxBounds = glm::max(maxBounds, bounds.second);
  }

  // Handle empty figures case
  if (minBounds.x == std::numeric_limits<float>::max()) {
    return {glm::vec3(0.0f), glm::vec3(100.0f)};
  }

  return {minBounds, maxBounds};
}

} // namespace import_export
} // namespace view
