/**
 * @file ImportExportDialog.cpp
 * @brief ImGUI dialog for import/export operations implementation
 */

#include "ImportExportDialog.hpp"
#include "ImportExportManager.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstring>

namespace view {
namespace import_export {

namespace {
/// Format names for display
const char *FORMAT_NAMES[] = {"Auto-detect", "DXF", "DWG", "SVG", "STL", "PNG"};

/// Number of format options
constexpr size_t FORMAT_COUNT = sizeof(FORMAT_NAMES) / sizeof(FORMAT_NAMES[0]);
} // namespace

ImportExportDialog::ImportExportDialog(ImportExportManager &manager)
    : manager_(manager) {}

bool ImportExportDialog::render() {
  if (!isOpen_) {
    return false;
  }

  // Set dialog size
  ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

  // Begin dialog
  std::string title = isImportMode_ ? "Import File" : "Export File";
  if (!ImGui::Begin(title.c_str(), &isOpen_, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return isOpen_;
  }

  // Mode toggle
  if (ImGui::RadioButton("Import", isImportMode_)) {
    isImportMode_ = true;
    setStatus("");
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Export", !isImportMode_)) {
    isImportMode_ = false;
    setStatus("");
  }

  ImGui::Separator();

  // Render mode-specific content
  if (isImportMode_) {
    renderImportMode();
  } else {
    renderExportMode();
  }

  ImGui::End();
  return isOpen_;
}

void ImportExportDialog::openImport() {
  isOpen_ = true;
  isImportMode_ = true;
  filePathBuffer_[0] = '\0';
  selectedFormatIndex_ = 0;
  statusMessage_[0] = '\0';
  hasError_ = false;
  figuresToExport_.clear();
}

void ImportExportDialog::openExport(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  isOpen_ = true;
  isImportMode_ = false;
  filePathBuffer_[0] = '\0';
  selectedFormatIndex_ = 0;
  statusMessage_[0] = '\0';
  hasError_ = false;
  figuresToExport_ = figures;
}

void ImportExportDialog::renderImportMode() {
  // File path input
  ImGui::Text("File Path:");
  ImGui::InputText("##FilePath", filePathBuffer_, sizeof(filePathBuffer_));

  // Format selection
  ImGui::Text("Format:");
  ImGui::Combo("##Format", &selectedFormatIndex_, FORMAT_NAMES,
               static_cast<int>(FORMAT_COUNT));

  // Supported formats info
  ImGui::Separator();
  ImGui::Text("Supported formats:");
  ImGui::BulletText("DXF - AutoCAD Drawing Exchange Format");
  ImGui::BulletText("SVG - Scalable Vector Graphics");
  ImGui::BulletText("STL - Stereolithography (3D printing)");

  // Status message
  if (statusMessage_[0] != '\0') {
    ImGui::Separator();
    if (hasError_) {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", statusMessage_);
    } else {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", statusMessage_);
    }
  }

  // Buttons
  ImGui::Separator();
  if (ImGui::Button("Import", ImVec2(120, 0))) {
    handleImport();
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(120, 0))) {
    isOpen_ = false;
  }
}

void ImportExportDialog::renderExportMode() {
  // File path input
  ImGui::Text("File Path:");
  ImGui::InputText("##FilePath", filePathBuffer_, sizeof(filePathBuffer_));

  // Format selection
  ImGui::Text("Format:");
  ImGui::Combo("##Format", &selectedFormatIndex_, FORMAT_NAMES,
               static_cast<int>(FORMAT_COUNT));

  // Figures count
  ImGui::Separator();
  ImGui::Text("Figures to export: %zu", figuresToExport_.size());

  // Supported formats info
  ImGui::Separator();
  ImGui::Text("Supported formats:");
  ImGui::BulletText("SVG - Scalable Vector Graphics");
  ImGui::BulletText("PNG - Portable Network Graphics (saved as PPM)");
  ImGui::BulletText("DXF - AutoCAD Drawing Exchange Format");
  ImGui::BulletText("STL - Stereolithography (3D printing)");

  // Status message
  if (statusMessage_[0] != '\0') {
    ImGui::Separator();
    if (hasError_) {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", statusMessage_);
    } else {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", statusMessage_);
    }
  }

  // Buttons
  ImGui::Separator();
  if (ImGui::Button("Export", ImVec2(120, 0))) {
    handleExport();
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(120, 0))) {
    isOpen_ = false;
  }
}

void ImportExportDialog::handleImport() {
  std::string filePath(filePathBuffer_);

  if (filePath.empty()) {
    setStatus("Please enter a file path", true);
    return;
  }

  FileFormat format = getSelectedFormat();

  // Auto-detect format from extension if selected
  if (format == FileFormat::Unknown) {
    format = detectFormatFromPath(filePath);
    if (format == FileFormat::Unknown) {
      setStatus("Could not detect file format. Please select manually.", true);
      return;
    }
  }

  // Perform import
  ImportResult result = manager_.importFromFile(filePath, format);

  if (result.success) {
    std::string msg = "Successfully imported " +
                      std::to_string(result.figures.size()) + " figure(s)";
    setStatus(msg, false);

    // Notify callback
    if (onImportSuccess_) {
      onImportSuccess_(result.figures);
    }

    // Close dialog on success
    isOpen_ = false;
  } else {
    setStatus(result.errorMessage, true);
  }
}

void ImportExportDialog::handleExport() {
  std::string filePath(filePathBuffer_);

  if (filePath.empty()) {
    setStatus("Please enter a file path", true);
    return;
  }

  if (figuresToExport_.empty()) {
    setStatus("No figures to export", true);
    return;
  }

  FileFormat format = getSelectedFormat();

  // Auto-detect format from extension if selected
  if (format == FileFormat::Unknown) {
    format = detectFormatFromPath(filePath);
    if (format == FileFormat::Unknown) {
      setStatus("Could not detect file format. Please select manually.", true);
      return;
    }
  }

  // Perform export
  ExportResult result =
      manager_.exportToFile(figuresToExport_, filePath, format);

  if (result.success) {
    std::string msg = "Successfully exported " +
                      std::to_string(result.figuresExported) + " figure(s)";
    setStatus(msg, false);

    // Show warnings if any
    if (!result.warnings.empty()) {
      // For now, just append first warning to status
      msg += " (" + result.warnings[0] + ")";
      setStatus(msg, false);
    }

    // Notify callback
    if (onExportSuccess_) {
      onExportSuccess_();
    }

    // Close dialog on success
    isOpen_ = false;
  } else {
    setStatus(result.errorMessage, true);
  }
}

FileFormat ImportExportDialog::getSelectedFormat() const {
  // Index 0 is "Auto-detect" which maps to Unknown
  if (selectedFormatIndex_ == 0) {
    return FileFormat::Unknown;
  }

  // Map index to format (offset by 1 for auto-detect option)
  switch (selectedFormatIndex_) {
  case 1:
    return FileFormat::DXF;
  case 2:
    return FileFormat::DWG;
  case 3:
    return FileFormat::SVG;
  case 4:
    return FileFormat::STL;
  case 5:
    return FileFormat::PNG;
  default:
    return FileFormat::Unknown;
  }
}

void ImportExportDialog::setStatus(const std::string &message, bool isError) {
  std::strncpy(statusMessage_, message.c_str(), sizeof(statusMessage_) - 1);
  statusMessage_[sizeof(statusMessage_) - 1] = '\0';
  hasError_ = isError;
}

std::string ImportExportDialog::getExtensionForFormat(FileFormat format) {
  switch (format) {
  case FileFormat::DXF:
    return ".dxf";
  case FileFormat::DWG:
    return ".dwg";
  case FileFormat::SVG:
    return ".svg";
  case FileFormat::STL:
    return ".stl";
  case FileFormat::PNG:
    return ".png";
  default:
    return "";
  }
}

std::string ImportExportDialog::getFilterForFormat(FileFormat format) {
  switch (format) {
  case FileFormat::DXF:
    return "DXF Files (*.dxf)";
  case FileFormat::DWG:
    return "DWG Files (*.dwg)";
  case FileFormat::SVG:
    return "SVG Files (*.svg)";
  case FileFormat::STL:
    return "STL Files (*.stl)";
  case FileFormat::PNG:
    return "PNG Files (*.png)";
  default:
    return "All Files (*.*)";
  }
}

} // namespace import_export
} // namespace view
