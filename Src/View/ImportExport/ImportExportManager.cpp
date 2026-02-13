/**
 * @file ImportExportManager.cpp
 * @brief Implementation of the ImportExportManager
 */

#include "ImportExportManager.hpp"

#include <spdlog/spdlog.h>

namespace view {
namespace import_export {

ImportExportManager::ImportExportManager() = default;

ImportExportManager::~ImportExportManager() = default;

void ImportExportManager::registerImporter(
    std::unique_ptr<IImporter> importer) {
  if (!importer) {
    spdlog::warn("ImportExportManager: Attempted to register null importer");
    return;
  }

  FileFormat format = importer->getFormat();
  spdlog::info("ImportExportManager: Registering importer for format: {}",
               getFormatDisplayName(format));
  importers_[format] = std::move(importer);
}

void ImportExportManager::registerExporter(
    std::unique_ptr<IExporter> exporter) {
  if (!exporter) {
    spdlog::warn("ImportExportManager: Attempted to register null exporter");
    return;
  }

  FileFormat format = exporter->getFormat();
  spdlog::info("ImportExportManager: Registering exporter for format: {}",
               getFormatDisplayName(format));
  exporters_[format] = std::move(exporter);
}

void ImportExportManager::registerDefaults() {
  spdlog::info(
      "ImportExportManager: Registering default importers and exporters");

  // Note: Actual importer/exporter implementations will be registered
  // when they are implemented. For now, we just log that defaults
  // would be registered.
  // TODO: Register DXFImporter, SVGImporter, STLImporter
  // TODO: Register DXFExporter, SVGExporter, STLExporter, PNGExporter
}

ImportResult ImportExportManager::importFromFile(const std::string &filePath,
                                                 FileFormat format) {
  ImportResult result;

  // Auto-detect format if not specified
  if (format == FileFormat::Unknown) {
    format = detectFormatFromPath(filePath);
    if (format == FileFormat::Unknown) {
      result.success = false;
      result.errorMessage =
          "Unable to detect file format from path: " + filePath;
      spdlog::error("ImportExportManager: {}", result.errorMessage);
      return result;
    }
  }

  // Find importer
  auto it = importers_.find(format);
  if (it == importers_.end()) {
    result.success = false;
    result.errorMessage =
        "No importer registered for format: " + getFormatDisplayName(format);
    spdlog::error("ImportExportManager: {}", result.errorMessage);
    return result;
  }

  // Check if file can be imported
  if (!it->second->canImport(filePath)) {
    result.success = false;
    result.errorMessage = "File cannot be imported: " + filePath;
    spdlog::error("ImportExportManager: {}", result.errorMessage);
    return result;
  }

  // Perform import
  reportProgress(0.0f, "Starting import...");
  spdlog::info("ImportExportManager: Importing from {}", filePath);

  result = it->second->import(filePath);

  if (result.success) {
    spdlog::info("ImportExportManager: Successfully imported {} figures",
                 result.figures.size());
    reportProgress(1.0f, "Import complete");
  } else {
    spdlog::error("ImportExportManager: Import failed: {}",
                  result.errorMessage);
  }

  return result;
}

ImportResult
ImportExportManager::importToModel(const std::string &filePath,
                                   std::shared_ptr<model::IModel> model,
                                   FileFormat format) {
  ImportResult result = importFromFile(filePath, format);

  if (result.success && model) {
    reportProgress(0.9f, "Adding figures to model...");

    // Add imported figures to the model
    for (const auto &figure : result.figures) {
      if (figure) {
        model->addFigure(figure->getPosition());
      }
    }

    spdlog::info("ImportExportManager: Added {} figures to model",
                 result.figures.size());
  }

  return result;
}

ExportResult ImportExportManager::exportToFile(
    const std::vector<std::shared_ptr<model::IFigure>> &figures,
    const std::string &filePath, FileFormat format) {

  ExportResult result;

  // Auto-detect format if not specified
  if (format == FileFormat::Unknown) {
    format = detectFormatFromPath(filePath);
    if (format == FileFormat::Unknown) {
      result.success = false;
      result.errorMessage =
          "Unable to detect file format from path: " + filePath;
      spdlog::error("ImportExportManager: {}", result.errorMessage);
      return result;
    }
  }

  // Find exporter
  auto it = exporters_.find(format);
  if (it == exporters_.end()) {
    result.success = false;
    result.errorMessage =
        "No exporter registered for format: " + getFormatDisplayName(format);
    spdlog::error("ImportExportManager: {}", result.errorMessage);
    return result;
  }

  // Check if figures are supported
  for (const auto &figure : figures) {
    if (figure && !it->second->supportsFigure(figure)) {
      result.warnings.push_back("Figure type not fully supported: " +
                                figure->getName());
      spdlog::warn("ImportExportManager: Figure type not fully supported: {}",
                   figure->getName());
    }
  }

  // Perform export
  reportProgress(0.0f, "Starting export...");
  spdlog::info("ImportExportManager: Exporting {} figures to {}",
               figures.size(), filePath);

  result = it->second->exportToFile(figures, filePath);

  if (result.success) {
    spdlog::info("ImportExportManager: Successfully exported {} figures",
                 result.figuresExported);
    reportProgress(1.0f, "Export complete");
  } else {
    spdlog::error("ImportExportManager: Export failed: {}",
                  result.errorMessage);
  }

  return result;
}

ExportResult
ImportExportManager::exportModel(std::shared_ptr<model::IModel> model,
                                 const std::string &filePath,
                                 FileFormat format) {
  ExportResult result;

  if (!model) {
    result.success = false;
    result.errorMessage = "Cannot export null model";
    spdlog::error("ImportExportManager: {}", result.errorMessage);
    return result;
  }

  // Collect all figures from the model
  std::vector<std::shared_ptr<model::IFigure>> figures;
  size_t count = model->getFigureCount();

  figures.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    auto figure = model->getFigure(static_cast<uint32_t>(i));
    if (figure) {
      figures.push_back(figure);
    }
  }

  return exportToFile(figures, filePath, format);
}

std::vector<FileFormat> ImportExportManager::getImportFormats() const {
  std::vector<FileFormat> formats;
  formats.reserve(importers_.size());

  for (const auto &pair : importers_) {
    formats.push_back(pair.first);
  }

  return formats;
}

std::vector<FileFormat> ImportExportManager::getExportFormats() const {
  std::vector<FileFormat> formats;
  formats.reserve(exporters_.size());

  for (const auto &pair : exporters_) {
    formats.push_back(pair.first);
  }

  return formats;
}

bool ImportExportManager::canImport(FileFormat format) const {
  return importers_.find(format) != importers_.end();
}

bool ImportExportManager::canExport(FileFormat format) const {
  return exporters_.find(format) != exporters_.end();
}

IImporter *ImportExportManager::getImporter(FileFormat format) const {
  auto it = importers_.find(format);
  return (it != importers_.end()) ? it->second.get() : nullptr;
}

IExporter *ImportExportManager::getExporter(FileFormat format) const {
  auto it = exporters_.find(format);
  return (it != exporters_.end()) ? it->second.get() : nullptr;
}

void ImportExportManager::setProgressCallback(ProgressCallback callback) {
  progressCallback_ = std::move(callback);
}

void ImportExportManager::reportProgress(float progress,
                                         const std::string &message) {
  if (progressCallback_) {
    progressCallback_(progress, message);
  }
}

} // namespace import_export
} // namespace view
