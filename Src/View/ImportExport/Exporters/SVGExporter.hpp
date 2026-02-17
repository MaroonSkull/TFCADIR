/**
 * @file SVGExporter.hpp
 * @brief SVG file format exporter
 *
 * Exports TFCADIR figures to SVG (Scalable Vector Graphics) format.
 * Supports all figure types: Triangle, Quad, Circle, Ngon, and Bezier curves.
 */

#pragma once

#include "View/ImportExport/ImporterExporterInterfaces.hpp"

#include <Model/FlatFigure.hpp>

#include <memory>
#include <string>

namespace view {
namespace import_export {

/**
 * @brief SVG file format exporter
 *
 * Converts TFCADIR figure objects to SVG format for use in
 * vector graphics applications and web publishing.
 */
class SVGExporter : public IExporter {
public:
  SVGExporter();
  ~SVGExporter() override = default;

  /**
   * @brief Export figures to an SVG file
   * @param figures Vector of figures to export
   * @param filePath Path to the output SVG file
   * @return ExportResult with success status and any messages
   */
  ExportResult
  exportToFile(const std::vector<std::shared_ptr<model::IFigure>> &figures,
               const std::string &filePath) override;

  /**
   * @brief Get the file format this exporter handles
   * @return FileFormat::SVG
   */
  FileFormat getFormat() const override { return FileFormat::SVG; }

  /**
   * @brief Get the exporter name
   * @return "SVG Exporter"
   */
  std::string getName() const override { return "SVG Exporter"; }

  /**
   * @brief Check if a figure type is supported for export
   * @param figure The figure to check
   * @return true for all figure types (SVG supports all)
   */
  bool
  supportsFigure(const std::shared_ptr<model::IFigure> &figure) const override;

private:
  /**
   * @brief Generate SVG content from figures
   * @param figures Vector of figures to convert
   * @return SVG content string
   */
  std::string
  generateSVG(const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Convert a Triangle figure to SVG element
   * @param figure The Triangle figure
   * @return SVG polygon element string
   */
  std::string triangleToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert a Quad figure to SVG element
   * @param figure The Quad figure
   * @return SVG polygon element string
   */
  std::string quadToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert a Circle figure to SVG element
   * @param figure The Circle figure
   * @return SVG circle element string
   */
  std::string circleToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert an Ngon figure to SVG element
   * @param figure The Ngon figure
   * @return SVG polygon element string
   */
  std::string ngonToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert a CurveBezier3 figure to SVG element
   * @param figure The CurveBezier3 figure
   * @return SVG path element string
   */
  std::string curveBezier3ToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert a CurveBezier4 figure to SVG element
   * @param figure The CurveBezier4 figure
   * @return SVG path element string
   */
  std::string curveBezier4ToSVG(const std::shared_ptr<model::IFigure> &figure);

  /**
   * @brief Convert a color to SVG format
   * @param color RGB color vector
   * @return SVG color string (rgb(r,g,b))
   */
  static std::string colorToSVG(const glm::vec3 &color);

  /**
   * @brief Calculate bounding box of all figures
   * @param figures Vector of figures
   * @return Pair of min and max bounds
   */
  static std::pair<glm::vec3, glm::vec3>
  calculateBounds(const std::vector<std::shared_ptr<model::IFigure>> &figures);
};

} // namespace import_export
} // namespace view
