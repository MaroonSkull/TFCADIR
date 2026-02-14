/**
 * @file FileFormats.hpp
 * @brief File format definitions for import/export functionality
 *
 * Defines file formats, format detection, and utility functions
 * for the TFCADIR CAD application import/export system.
 */

#pragma once

#include <string>
#include <vector>

namespace view {
namespace import_export {

/**
 * @brief Enumeration of supported file formats for import/export
 *
 * Each format has specific capabilities for import and/or export operations.
 * Some formats support both directions (DXF, SVG, STL), while others
 * are export-only (PNG).
 */
enum class FileFormat {
  DXF,    ///< AutoCAD Drawing Exchange Format - full CAD support
  DWG,    ///< AutoCAD Drawing format - limited support (read-only)
  SVG,    ///< Scalable Vector Graphics - vector graphics format
  STL,    ///< Stereolithography format - 3D mesh export
  PNG,    ///< Portable Network Graphics - raster image export
  Unknown ///< Unknown or unsupported format
};

/**
 * @brief File operation direction (import or export)
 */
enum class FileOperation {
  Import, ///< Import operation - reading from file
  Export  ///< Export operation - writing to file
};

/**
 * @brief File format capability flags
 */
struct FormatCapabilities {
  bool canImport{false};        ///< Format supports import
  bool canExport{false};        ///< Format supports export
  bool isBinary{false};         ///< Format is binary (vs text)
  bool supports3D{false};       ///< Format supports 3D geometry
  bool supportsLayers{false};   ///< Format supports layer information
  bool supportsMetadata{false}; ///< Format supports metadata/properties
};

/**
 * @brief Get file extension for a given format
 * @param format The file format
 * @return File extension without dot (e.g., "dxf", "svg")
 */
std::string getFileExtension(FileFormat format);

/**
 * @brief Detect file format from file path extension
 * @param filePath Path to the file
 * @return Detected file format, or FileFormat::Unknown if not recognized
 */
FileFormat detectFormatFromPath(const std::string &filePath);

/**
 * @brief Get human-readable name for a file format
 * @param format The file format
 * @return Human-readable name (e.g., "DXF Drawing", "SVG Vector Graphics")
 */
std::string getFormatDisplayName(FileFormat format);

/**
 * @brief Get capabilities for a file format
 * @param format The file format
 * @return FormatCapabilities structure with capability flags
 */
FormatCapabilities getFormatCapabilities(FileFormat format);

/**
 * @brief Get all supported formats for a given operation
 * @param operation Import or Export operation
 * @return Vector of supported file formats
 */
std::vector<FileFormat> getSupportedFormats(FileOperation operation);

/**
 * @brief Check if a format supports the specified operation
 * @param format The file format to check
 * @param operation Import or Export operation
 * @return true if the format supports the operation
 */
bool supportsOperation(FileFormat format, FileOperation operation);

/**
 * @brief Get file filter string for file dialogs
 * @param format The file format
 * @return Filter string for ImGui file dialog (e.g., "*.dxf")
 */
std::string getFileFilter(FileFormat format);

/**
 * @brief Get all file filters for a given operation
 * @param operation Import or Export operation
 * @return Combined filter string for file dialogs
 */
std::string getAllFileFilters(FileOperation operation);

} // namespace import_export
} // namespace view
