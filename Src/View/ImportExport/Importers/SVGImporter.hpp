/**
 * @file SVGImporter.hpp
 * @brief SVG file format importer
 *
 * Imports SVG (Scalable Vector Graphics) files and converts them
 * to TFCADIR figure objects. Supports basic SVG elements including
 * circles, rectangles, lines, and paths.
 */

#pragma once

#include "View/ImportExport/ImporterExporterInterfaces.hpp"

#include <Model/FlatFigure.hpp>

#include <memory>
#include <string>

namespace view {
namespace import_export {

/**
 * @brief SVG file format importer
 *
 * Parses SVG files and extracts 2D geometric elements.
 * Supports the following SVG elements:
 * - circle: Converted to Circle figures
 * - rect: Converted to Quad figures
 * - line: Converted to line segments
 * - path: Basic path parsing for simple shapes
 *
 * Note: This is a simplified implementation that handles
 * basic SVG structure. Full SVG specification support would
 * require a dedicated library like nanosvg or lunasvg.
 */
class SVGImporter : public IImporter {
public:
  SVGImporter();
  ~SVGImporter() override = default;

  /**
   * @brief Import figures from an SVG file
   * @param filePath Path to the SVG file
   * @return ImportResult with imported figures and status
   */
  ImportResult import(const std::string &filePath) override;

  /**
   * @brief Get the file format this importer handles
   * @return FileFormat::SVG
   */
  FileFormat getFormat() const override { return FileFormat::SVG; }

  /**
   * @brief Get the importer name
   * @return "SVG Importer"
   */
  std::string getName() const override { return "SVG Importer"; }

  /**
   * @brief Check if a file can be imported as SVG
   * @param filePath Path to check
   * @return true if the file appears to be a valid SVG file
   */
  bool canImport(const std::string &filePath) const override;

private:
  /**
   * @brief Parse SVG content and extract figures
   * @param content SVG file content
   * @param result ImportResult to populate with figures
   * @return true if parsing succeeded
   */
  bool parseSVG(const std::string &content, ImportResult &result);

  /**
   * @brief Parse a circle element
   * @param element SVG element string
   * @param result ImportResult to add figure to
   */
  void parseCircle(const std::string &element, ImportResult &result);

  /**
   * @brief Parse a rectangle element
   * @param element SVG element string
   * @param result ImportResult to add figure to
   */
  void parseRect(const std::string &element, ImportResult &result);

  /**
   * @brief Parse a line element
   * @param element SVG element string
   * @param result ImportResult to add figure to
   */
  void parseLine(const std::string &element, ImportResult &result);

  /**
   * @brief Extract an attribute value from an SVG element
   * @param element SVG element string
   * @param attr Attribute name to find
   * @param defaultValue Default value if not found
   * @return Attribute value as float
   */
  static float getAttribute(const std::string &element, const std::string &attr,
                            float defaultValue = 0.0f);

  /**
   * @brief Extract a string attribute value from an SVG element
   * @param element SVG element string
   * @param attr Attribute name to find
   * @return Attribute value as string, empty if not found
   */
  static std::string getStringAttribute(const std::string &element,
                                        const std::string &attr);
};

} // namespace import_export
} // namespace view
