/**
 * @file FileFormats.cpp
 * @brief Implementation of file format utilities
 */

#include "FileFormats.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace view {
namespace import_export {

std::string getFileExtension(FileFormat format) {
  switch (format) {
  case FileFormat::DXF:
    return "dxf";
  case FileFormat::DWG:
    return "dwg";
  case FileFormat::SVG:
    return "svg";
  case FileFormat::STL:
    return "stl";
  case FileFormat::PNG:
    return "png";
  case FileFormat::Unknown:
  default:
    return "";
  }
}

FileFormat detectFormatFromPath(const std::string &filePath) {
  // Find the last dot in the path
  size_t dotPos = filePath.rfind('.');
  if (dotPos == std::string::npos || dotPos == filePath.length() - 1) {
    return FileFormat::Unknown;
  }

  // Extract extension and convert to lowercase
  std::string extension = filePath.substr(dotPos + 1);
  std::transform(
      extension.begin(), extension.end(), extension.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  // Match extension to format
  if (extension == "dxf") {
    return FileFormat::DXF;
  }
  if (extension == "dwg") {
    return FileFormat::DWG;
  }
  if (extension == "svg") {
    return FileFormat::SVG;
  }
  if (extension == "stl") {
    return FileFormat::STL;
  }
  if (extension == "png") {
    return FileFormat::PNG;
  }

  return FileFormat::Unknown;
}

std::string getFormatDisplayName(FileFormat format) {
  switch (format) {
  case FileFormat::DXF:
    return "DXF Drawing";
  case FileFormat::DWG:
    return "DWG Drawing";
  case FileFormat::SVG:
    return "SVG Vector Graphics";
  case FileFormat::STL:
    return "STL 3D Mesh";
  case FileFormat::PNG:
    return "PNG Image";
  case FileFormat::Unknown:
  default:
    return "Unknown Format";
  }
}

FormatCapabilities getFormatCapabilities(FileFormat format) {
  switch (format) {
  case FileFormat::DXF:
    return {true, true, false, false, true, true};
  case FileFormat::DWG:
    return {true, false, true, false, true, true}; // Import only, binary
  case FileFormat::SVG:
    return {true, true, false, false, true, true};
  case FileFormat::STL:
    return {true, true, true, true, false, false}; // Binary, 3D only
  case FileFormat::PNG:
    return {false, true, true, false, false, true}; // Export only, binary
  case FileFormat::Unknown:
  default:
    return {false, false, false, false, false, false};
  }
}

std::vector<FileFormat> getSupportedFormats(FileOperation operation) {
  std::vector<FileFormat> formats;

  switch (operation) {
  case FileOperation::Import:
    formats = {FileFormat::DXF, FileFormat::DWG, FileFormat::SVG,
               FileFormat::STL};
    break;
  case FileOperation::Export:
    formats = {FileFormat::DXF, FileFormat::SVG, FileFormat::STL,
               FileFormat::PNG};
    break;
  }

  return formats;
}

bool supportsOperation(FileFormat format, FileOperation operation) {
  FormatCapabilities caps = getFormatCapabilities(format);
  return (operation == FileOperation::Import) ? caps.canImport : caps.canExport;
}

std::string getFileFilter(FileFormat format) {
  std::string ext = getFileExtension(format);
  if (ext.empty()) {
    return "*.*";
  }
  return "*." + ext;
}

std::string getAllFileFilters(FileOperation operation) {
  std::vector<FileFormat> formats = getSupportedFormats(operation);
  std::ostringstream oss;

  // Create "All Supported Formats" filter
  oss << "All Supported Formats (";
  for (size_t i = 0; i < formats.size(); ++i) {
    if (i > 0) {
      oss << ", ";
    }
    oss << "*." << getFileExtension(formats[i]);
  }
  oss << "){";
  for (size_t i = 0; i < formats.size(); ++i) {
    if (i > 0) {
      oss << ",";
    }
    oss << "*." << getFileExtension(formats[i]);
  }
  oss << "},";

  // Add individual format filters
  for (FileFormat format : formats) {
    oss << getFormatDisplayName(format) << " (*." << getFileExtension(format)
        << "){*." << getFileExtension(format) << "},";
  }

  // Add "All Files" filter
  oss << "All Files (*.*){*.*}";

  return oss.str();
}

} // namespace import_export
} // namespace view
