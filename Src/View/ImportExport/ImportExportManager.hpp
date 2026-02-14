/**
 * @file ImportExportManager.hpp
 * @brief Central manager for import/export operations
 *
 * The ImportExportManager coordinates all import and export operations,
 * managing format-specific importers and exporters and providing a
 * unified interface for the application.
 */

#pragma once

#include "FileFormats.hpp"
#include "ImporterExporterInterfaces.hpp"

#include <Model/IModel.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace view {
namespace import_export {

/**
 * @brief Progress callback type for long-running operations
 * @param progress Progress value from 0.0 to 1.0
 * @param message Current operation description
 */
using ProgressCallback =
    std::function<void(float progress, const std::string &message)>;

/**
 * @brief Central manager for all import/export operations
 *
 * This class manages all registered importers and exporters, provides
 * format detection, and coordinates import/export operations with the
 * model layer.
 *
 * Usage:
 * @code
 * ImportExportManager manager;
 * manager.registerImporter(std::make_unique<DXFImporter>());
 * manager.registerExporter(std::make_unique<SVGExporter>());
 *
 * auto result = manager.importFromFile("/path/to/file.dxf", FileFormat::DXF);
 * if (result.success) {
 *     // Add figures to model
 * }
 * @endcode
 */
class ImportExportManager {
public:
  ImportExportManager();
  ~ImportExportManager();

  // Prevent copying
  ImportExportManager(const ImportExportManager &) = delete;
  ImportExportManager &operator=(const ImportExportManager &) = delete;

  // === Importer/Exporter Registration ===

  /**
   * @brief Register an importer for a specific format
   * @param importer Unique pointer to the importer
   * @note Only one importer per format is supported; later registrations
   *       replace earlier ones
   */
  void registerImporter(std::unique_ptr<IImporter> importer);

  /**
   * @brief Register an exporter for a specific format
   * @param exporter Unique pointer to the exporter
   * @note Only one exporter per format is supported; later registrations
   *       replace earlier ones
   */
  void registerExporter(std::unique_ptr<IExporter> exporter);

  /**
   * @brief Register all default importers and exporters
   *
   * Registers built-in importers (DXF, SVG, STL) and exporters
   * (DXF, SVG, STL, PNG).
   */
  void registerDefaults();

  // === Import Operations ===

  /**
   * @brief Import figures from a file
   * @param filePath Path to the file to import
   * @param format File format (auto-detected if Unknown)
   * @return ImportResult with imported figures and status
   */
  ImportResult importFromFile(const std::string &filePath,
                              FileFormat format = FileFormat::Unknown);

  /**
   * @brief Import figures and add them directly to a model
   * @param filePath Path to the file to import
   * @param model The model to add figures to
   * @param format File format (auto-detected if Unknown)
   * @return ImportResult with status (figures are added to model)
   */
  ImportResult importToModel(const std::string &filePath,
                             std::shared_ptr<model::IModel> model,
                             FileFormat format = FileFormat::Unknown);

  // === Export Operations ===

  /**
   * @brief Export figures to a file
   * @param figures Vector of figures to export
   * @param filePath Path to the output file
   * @param format File format (auto-detected if Unknown)
   * @return ExportResult with status
   */
  ExportResult
  exportToFile(const std::vector<std::shared_ptr<model::IFigure>> &figures,
               const std::string &filePath,
               FileFormat format = FileFormat::Unknown);

  /**
   * @brief Export all figures from a model to a file
   * @param model The model to export figures from
   * @param filePath Path to the output file
   * @param format File format (auto-detected if Unknown)
   * @return ExportResult with status
   */
  ExportResult exportModel(std::shared_ptr<model::IModel> model,
                           const std::string &filePath,
                           FileFormat format = FileFormat::Unknown);

  // === Format Information ===

  /**
   * @brief Get all registered import formats
   * @return Vector of supported import formats
   */
  std::vector<FileFormat> getImportFormats() const;

  /**
   * @brief Get all registered export formats
   * @return Vector of supported export formats
   */
  std::vector<FileFormat> getExportFormats() const;

  /**
   * @brief Check if a format is supported for import
   * @param format The format to check
   * @return true if an importer is registered for this format
   */
  bool canImport(FileFormat format) const;

  /**
   * @brief Check if a format is supported for export
   * @param format The format to check
   * @return true if an exporter is registered for this format
   */
  bool canExport(FileFormat format) const;

  /**
   * @brief Get the importer for a format
   * @param format The format to get importer for
   * @return Pointer to importer, or nullptr if not registered
   */
  IImporter *getImporter(FileFormat format) const;

  /**
   * @brief Get the exporter for a format
   * @param format The format to get exporter for
   * @return Pointer to exporter, or nullptr if not registered
   */
  IExporter *getExporter(FileFormat format) const;

  // === Progress Callback ===

  /**
   * @brief Set a progress callback for long-running operations
   * @param callback Function to call with progress updates
   */
  void setProgressCallback(ProgressCallback callback);

private:
  /// Map of file formats to importers
  std::map<FileFormat, std::unique_ptr<IImporter>> importers_;

  /// Map of file formats to exporters
  std::map<FileFormat, std::unique_ptr<IExporter>> exporters_;

  /// Optional progress callback
  ProgressCallback progressCallback_;

  /**
   * @brief Report progress if callback is set
   * @param progress Progress value (0.0 to 1.0)
   * @param message Current operation description
   */
  void reportProgress(float progress, const std::string &message);
};

} // namespace import_export
} // namespace view
