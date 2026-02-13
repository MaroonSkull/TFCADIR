/**
 * @file ImporterExporterInterfaces.hpp
 * @brief Interface definitions for importers and exporters
 *
 * Defines the abstract interfaces IImporter and IExporter that all
 * format-specific importers and exporters must implement.
 */

#pragma once

#include "FileFormats.hpp"

#include <Model/IModel.hpp>

#include <memory>
#include <string>
#include <vector>

namespace view {
namespace import_export {

/**
 * @brief Result structure for import operations
 *
 * Contains the imported figures and any error/warning messages.
 */
struct ImportResult {
  std::vector<std::shared_ptr<model::IFigure>>
      figures;                       ///< Successfully imported figures
  bool success{false};               ///< Whether the import succeeded
  std::string errorMessage;          ///< Error message if failed
  std::vector<std::string> warnings; ///< Non-fatal warnings
};

/**
 * @brief Result structure for export operations
 *
 * Contains success status and any error/warning messages.
 */
struct ExportResult {
  bool success{false};               ///< Whether the export succeeded
  std::string errorMessage;          ///< Error message if failed
  std::vector<std::string> warnings; ///< Non-fatal warnings
  size_t figuresExported{0};         ///< Number of figures exported
};

/**
 * @brief Abstract interface for file importers
 *
 * All format-specific importers (DXF, SVG, STL) must implement this interface.
 * The importer is responsible for reading a file and converting its contents
 * to TFCADIR figure objects.
 */
class IImporter {
public:
  virtual ~IImporter() = default;

  /**
   * @brief Import figures from a file
   * @param filePath Path to the file to import
   * @return ImportResult containing imported figures and status
   */
  virtual ImportResult import(const std::string &filePath) = 0;

  /**
   * @brief Get the file format this importer handles
   * @return The supported FileFormat
   */
  virtual FileFormat getFormat() const = 0;

  /**
   * @brief Get a human-readable name for this importer
   * @return Name string (e.g., "DXF Importer")
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Check if a file can be imported by this importer
   * @param filePath Path to the file to check
   * @return true if the file appears to be valid for this format
   */
  virtual bool canImport(const std::string &filePath) const = 0;
};

/**
 * @brief Abstract interface for file exporters
 *
 * All format-specific exporters (DXF, SVG, STL, PNG) must implement this
 * interface. The exporter is responsible for converting TFCADIR figure objects
 * to the target file format.
 */
class IExporter {
public:
  virtual ~IExporter() = default;

  /**
   * @brief Export figures to a file
   * @param figures Vector of figures to export
   * @param filePath Path to the output file
   * @return ExportResult containing success status and any messages
   */
  virtual ExportResult
  exportToFile(const std::vector<std::shared_ptr<model::IFigure>> &figures,
               const std::string &filePath) = 0;

  /**
   * @brief Get the file format this exporter handles
   * @return The supported FileFormat
   */
  virtual FileFormat getFormat() const = 0;

  /**
   * @brief Get a human-readable name for this exporter
   * @return Name string (e.g., "SVG Exporter")
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Check if a figure type is supported for export
   * @param figure The figure to check
   * @return true if the figure can be exported to this format
   */
  virtual bool
  supportsFigure(const std::shared_ptr<model::IFigure> &figure) const = 0;
};

} // namespace import_export
} // namespace view
