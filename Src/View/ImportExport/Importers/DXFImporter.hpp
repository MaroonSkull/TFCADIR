/**
 * @file DXFImporter.hpp
 * @brief DXF/DWG file format importer
 *
 * Imports DXF (Drawing Exchange Format) files and converts them
 * to TFCADIR figure objects. Supports basic 2D entities including
 * lines, circles, arcs, and polylines.
 */

#pragma once

#include "View/ImportExport/ImporterExporterInterfaces.hpp"

#include <Model/FlatFigure.hpp>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace view {
namespace import_export {

/**
 * @brief DXF file format importer
 *
 * Parses DXF files and extracts 2D geometric entities.
 * Supports the following DXF entities:
 * - LINE: Converted to line segments
 * - CIRCLE: Converted to Circle figures
 * - ARC: Converted to Circle figures (full circle approximation)
 * - LWPOLYLINE: Converted to line segments
 * - POLYLINE: Converted to line segments
 *
 * Note: This is a simplified implementation that handles
 * basic DXF structure. Full DXF specification support would
 * require a dedicated library like libdxfrw or dxflib.
 */
class DXFImporter : public IImporter {
public:
  DXFImporter();
  ~DXFImporter() override = default;

  /**
   * @brief Import figures from a DXF file
   * @param filePath Path to the DXF file
   * @return ImportResult with imported figures and status
   */
  ImportResult import(const std::string &filePath) override;

  /**
   * @brief Get the file format this importer handles
   * @return FileFormat::DXF
   */
  FileFormat getFormat() const override { return FileFormat::DXF; }

  /**
   * @brief Get the importer name
   * @return "DXF Importer"
   */
  std::string getName() const override { return "DXF Importer"; }

  /**
   * @brief Check if a file can be imported as DXF
   * @param filePath Path to check
   * @return true if the file appears to be a valid DXF file
   */
  bool canImport(const std::string &filePath) const override;

private:
  /**
   * @brief DXF entity types
   */
  enum class DXFEntityType { Line, Circle, Arc, Polyline, LWPolyline, Unknown };

  /**
   * @brief Parsed DXF entity data
   */
  struct DXFEntity {
    DXFEntityType type{DXFEntityType::Unknown};
    float x1{0.0f}, y1{0.0f}, z1{0.0f};            ///< Start point
    float x2{0.0f}, y2{0.0f}, z2{0.0f};            ///< End point
    float centerX{0.0f}, centerY{0.0f};            ///< Center for circle/arc
    float radius{0.0f};                            ///< Radius for circle/arc
    std::vector<std::pair<float, float>> vertices; ///< Polyline vertices
    int layerIndex{0};                             ///< Layer number
  };

  /**
   * @brief Parse a DXF file and extract entities
   * @param file Input file stream
   * @param entities Vector to store parsed entities
   * @return true if parsing succeeded
   */
  bool parseDXF(std::ifstream &file, std::vector<DXFEntity> &entities);

  /**
   * @brief Read a DXF group code and value
   * @param file Input file stream
   * @param groupCode Output group code
   * @param value Output value string
   * @return true if read succeeded
   */
  static bool readGroupCode(std::ifstream &file, int &groupCode,
                            std::string &value);

  /**
   * @brief Convert a DXF entity to a TFCADIR figure
   * @param entity The DXF entity to convert
   * @return Shared pointer to the created figure, or nullptr if conversion
   * failed
   */
  std::shared_ptr<model::IFigure> convertEntity(const DXFEntity &entity);

  /**
   * @brief Skip to a specific section in the DXF file
   * @param file Input file stream
   * @param sectionName Name of the section to find
   * @return true if section was found
   */
  bool skipToSection(std::ifstream &file, const std::string &sectionName);
};

} // namespace import_export
} // namespace view
