/**
 * @file ImportExportDialog.hpp
 * @brief ImGUI dialog for import/export operations
 *
 * Provides a user interface for importing and exporting figures
 * in various file formats.
 */

#pragma once

#include "View/ImportExport/FileFormats.hpp"
#include "View/ImportExport/ImporterExporterInterfaces.hpp"

#include <Model/FlatFigure.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace view {
namespace import_export {

class ImportExportManager;

/**
 * @brief ImGUI dialog for import/export operations
 *
 * Provides a modal dialog with:
 * - File path input
 * - Format selection dropdown
 * - Import/Export mode toggle
 * - Preview of supported formats
 */
class ImportExportDialog {
public:
  /**
   * @brief Constructor
   * @param manager Reference to the ImportExportManager
   */
  explicit ImportExportDialog(ImportExportManager &manager);

  ~ImportExportDialog() = default;

  /**
   * @brief Render the dialog
   * @return true if dialog is open and should continue rendering
   */
  bool render();

  /**
   * @brief Open the dialog in import mode
   */
  void openImport();

  /**
   * @brief Open the dialog in export mode
   * @param figures Figures to export (for export mode)
   */
  void openExport(const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Check if dialog is currently open
   * @return true if dialog is open
   */
  bool isOpen() const { return isOpen_; }

  /**
   * @brief Set callback for successful import
   * @param callback Function to call with imported figures
   */
  void setOnImportSuccess(
      std::function<void(const std::vector<std::shared_ptr<model::IFigure>> &)>
          callback) {
    onImportSuccess_ = std::move(callback);
  }

  /**
   * @brief Set callback for successful export
   * @param callback Function to call after successful export
   */
  void setOnExportSuccess(std::function<void()> callback) {
    onExportSuccess_ = std::move(callback);
  }

private:
  ImportExportManager &manager_;
  bool isOpen_{false};
  bool isImportMode_{true};

  // Dialog state
  char filePathBuffer_[512]{'\0'};
  int selectedFormatIndex_{0};
  char statusMessage_[256]{'\0'};
  bool hasError_{false};

  // Figures to export (for export mode)
  std::vector<std::shared_ptr<model::IFigure>> figuresToExport_;

  // Callbacks
  std::function<void(const std::vector<std::shared_ptr<model::IFigure>> &)>
      onImportSuccess_;
  std::function<void()> onExportSuccess_;

  /**
   * @brief Render import mode UI
   */
  void renderImportMode();

  /**
   * @brief Render export mode UI
   */
  void renderExportMode();

  /**
   * @brief Handle import button click
   */
  void handleImport();

  /**
   * @brief Handle export button click
   */
  void handleExport();

  /**
   * @brief Get currently selected file format
   * @return Selected FileFormat
   */
  FileFormat getSelectedFormat() const;

  /**
   * @brief Update status message
   * @param message Message to display
   * @param isError Whether this is an error message
   */
  void setStatus(const std::string &message, bool isError = false);

  /**
   * @brief Get file extension for format
   * @param format File format
   * @return File extension (e.g., ".svg")
   */
  static std::string getExtensionForFormat(FileFormat format);

  /**
   * @brief Get format filter string for file dialogs
   * @param format File format
   * @return Filter string (e.g., "SVG Files (*.svg)")
   */
  static std::string getFilterForFormat(FileFormat format);
};

} // namespace import_export
} // namespace view
