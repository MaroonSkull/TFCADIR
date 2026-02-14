/**
 * @file SettingsManager.cpp
 * @brief Implementation of the Settings persistence manager
 */

#include "SettingsManager.hpp"
#include "DisplaySettings.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace view {
namespace settings {

SettingsManager &SettingsManager::Instance() {
  static SettingsManager instance;
  return instance;
}

SettingsManager::SettingsManager() : gridConfig_(), displayConfig_() {
  LoadDefaults();
}

void SettingsManager::LoadFromFile(const std::string &filepath) {
  // Check if file exists
  if (!std::filesystem::exists(filepath)) {
    spdlog::warn("Settings file not found: {}. Creating default settings.",
                 filepath);

    // Ensure parent directory exists
    std::filesystem::path path(filepath);
    std::filesystem::path parentPath = path.parent_path();
    if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
      std::error_code ec;
      if (!std::filesystem::create_directories(parentPath, ec)) {
        throw std::runtime_error("Failed to create settings directory: " +
                                 parentPath.string() + " - " + ec.message());
      }
      spdlog::info("Created settings directory: {}", parentPath.string());
    }

    // Create default settings file
    SaveToFile(filepath);
    spdlog::info("Created default settings file: {}", filepath);
    return;
  }

  std::ifstream file(filepath);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open settings file: " + filepath);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  // Parse the YAML-like content
  // Simple parsing for our settings format
  try {
    // Parse grid settings
    ParseGridConfig(content);
    ParseDisplayConfig(content);

    lastFilePath_ = filepath;
    hasUnsavedChanges_ = false;

    spdlog::info("Settings loaded from: {}", filepath);
  } catch (const std::exception &e) {
    throw std::runtime_error("Failed to parse settings file: " +
                             std::string(e.what()));
  }
}

void SettingsManager::SaveToFile(const std::string &filepath) {
  std::ofstream file(filepath);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to create settings file: " + filepath);
  }

  // Write YAML-like content
  file << "# TFCADIR Application Settings\n";
  file << "# Auto-generated - do not edit manually unless necessary\n\n";

  file << "grid:\n";
  file << SerializeGridConfig();

  file << "\ndisplay:\n";
  file << SerializeDisplayConfig();

  file.close();

  lastFilePath_ = filepath;
  hasUnsavedChanges_ = false;

  spdlog::info("Settings saved to: {}", filepath);
}

GridConfig SettingsManager::GetGridConfig() const { return gridConfig_; }

void SettingsManager::SetGridConfig(const GridConfig &config) {
  if (gridConfig_ != config) {
    gridConfig_ = config;
    hasUnsavedChanges_ = true;
  }
}

DisplayConfig SettingsManager::GetDisplayConfig() const {
  return displayConfig_;
}

void SettingsManager::SetDisplayConfig(const DisplayConfig &config) {
  if (displayConfig_ != config) {
    displayConfig_ = config;
    hasUnsavedChanges_ = true;
  }
}

AppSettings SettingsManager::GetAppSettings() const {
  AppSettings settings;
  settings.gridConfig = gridConfig_;
  settings.displayConfig = displayConfig_;
  return settings;
}

void SettingsManager::SetAppSettings(const AppSettings &settings) {
  SetGridConfig(settings.gridConfig);
  SetDisplayConfig(settings.displayConfig);
}

void SettingsManager::ApplySettings() {
  // Apply display theme
  DisplaySettings::ApplyTheme(displayConfig_);

  spdlog::info("Settings applied to application");
}

void SettingsManager::ResetToDefaults() {
  gridConfig_ = GridConfig();
  displayConfig_ = DisplayConfig();
  hasUnsavedChanges_ = true;

  spdlog::info("Settings reset to defaults");
}

bool SettingsManager::HasUnsavedChanges() const { return hasUnsavedChanges_; }

void SettingsManager::MarkAsSaved() { hasUnsavedChanges_ = false; }

void SettingsManager::LoadDefaults() {
  gridConfig_ = GridConfig();
  displayConfig_ = DisplayConfig();
}

void SettingsManager::ParseGridConfig(const std::string &content) {
  // Simple YAML-like parsing for grid settings
  // Look for "grid:" section
  size_t gridPos = content.find("grid:");
  if (gridPos == std::string::npos) {
    return; // No grid section found, use defaults
  }

  // Find the next section or end of file
  size_t nextSection = content.find("\n\n", gridPos);
  if (nextSection == std::string::npos) {
    nextSection = content.length();
  }

  std::string gridSection = content.substr(gridPos, nextSection - gridPos);

  // Parse individual settings
  auto parseBool = [&gridSection](const std::string &key,
                                  bool defaultValue) -> bool {
    size_t pos = gridSection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        gridSection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    std::string value = gridSection.substr(valueStart, 10);
    return value.find("true") != std::string::npos ||
           value.find("True") != std::string::npos ||
           value.find("1") != std::string::npos;
  };

  auto parseFloat = [&gridSection](const std::string &key,
                                   float defaultValue) -> float {
    size_t pos = gridSection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        gridSection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    try {
      return std::stof(gridSection.substr(valueStart, 20));
    } catch (...) {
      return defaultValue;
    }
  };

  auto parseInt = [&gridSection](const std::string &key,
                                 int defaultValue) -> int {
    size_t pos = gridSection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        gridSection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    try {
      return std::stoi(gridSection.substr(valueStart, 20));
    } catch (...) {
      return defaultValue;
    }
  };

  auto parseVec4 = [&gridSection](const std::string &key,
                                  glm::vec4 defaultValue) -> glm::vec4 {
    size_t pos = gridSection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t bracketStart = gridSection.find("[", pos);
    if (bracketStart == std::string::npos)
      return defaultValue;
    size_t bracketEnd = gridSection.find("]", bracketStart);
    if (bracketEnd == std::string::npos)
      return defaultValue;

    std::string values =
        gridSection.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    glm::vec4 result = defaultValue;
    size_t start = 0;
    for (int i = 0; i < 4 && start < values.length(); ++i) {
      size_t comma = values.find(",", start);
      if (comma == std::string::npos)
        comma = values.length();
      try {
        result[i] = std::stof(values.substr(start, comma - start));
      } catch (...) {
        result[i] = defaultValue[i];
      }
      start = comma + 1;
    }
    return result;
  };

  auto parseString = [&gridSection](const std::string &key,
                                    const std::string &defaultValue) {
    size_t pos = gridSection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        gridSection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    size_t valueEnd = gridSection.find_first_of("\n\r", valueStart);
    if (valueEnd == std::string::npos)
      valueEnd = gridSection.length();
    std::string value = gridSection.substr(valueStart, valueEnd - valueStart);
    // Remove quotes if present
    if (!value.empty() && value.front() == '"')
      value.erase(0, 1);
    if (!value.empty() && value.back() == '"')
      value.pop_back();
    return value;
  };

  // Parse grid settings
  gridConfig_.showGrid = parseBool("showGrid", gridConfig_.showGrid);
  gridConfig_.showMinorGridLines =
      parseBool("showMinorGridLines", gridConfig_.showMinorGridLines);
  gridConfig_.majorGridSpacing =
      parseFloat("majorGridSpacing", gridConfig_.majorGridSpacing);
  gridConfig_.minorDivisions =
      parseInt("minorDivisions", gridConfig_.minorDivisions);
  gridConfig_.majorGridColor =
      parseVec4("majorGridColor", gridConfig_.majorGridColor);
  gridConfig_.minorGridColor =
      parseVec4("minorGridColor", gridConfig_.minorGridColor);
  gridConfig_.gridOpacity = parseFloat("gridOpacity", gridConfig_.gridOpacity);

  std::string gridTypeStr = parseString("gridType", "Cartesian");
  try {
    gridConfig_.gridType = stringToGridType(gridTypeStr);
  } catch (...) {
    gridConfig_.gridType = GridType::Cartesian;
  }
}

void SettingsManager::ParseDisplayConfig(const std::string &content) {
  // Simple YAML-like parsing for display settings
  size_t displayPos = content.find("display:");
  if (displayPos == std::string::npos) {
    return; // No display section found, use defaults
  }

  size_t nextSection = content.find("\n\n", displayPos);
  if (nextSection == std::string::npos) {
    nextSection = content.length();
  }

  std::string displaySection =
      content.substr(displayPos, nextSection - displayPos);

  // Helper functions (same as above)
  auto parseBool = [&displaySection](const std::string &key,
                                     bool defaultValue) -> bool {
    size_t pos = displaySection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        displaySection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    std::string value = displaySection.substr(valueStart, 10);
    return value.find("true") != std::string::npos ||
           value.find("True") != std::string::npos ||
           value.find("1") != std::string::npos;
  };

  auto parseInt = [&displaySection](const std::string &key,
                                    int defaultValue) -> int {
    size_t pos = displaySection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        displaySection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    try {
      return std::stoi(displaySection.substr(valueStart, 20));
    } catch (...) {
      return defaultValue;
    }
  };

  auto parseVec4 = [&displaySection](const std::string &key,
                                     glm::vec4 defaultValue) -> glm::vec4 {
    size_t pos = displaySection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t bracketStart = displaySection.find("[", pos);
    if (bracketStart == std::string::npos)
      return defaultValue;
    size_t bracketEnd = displaySection.find("]", bracketStart);
    if (bracketEnd == std::string::npos)
      return defaultValue;

    std::string values =
        displaySection.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    glm::vec4 result = defaultValue;
    size_t start = 0;
    for (int i = 0; i < 4 && start < values.length(); ++i) {
      size_t comma = values.find(",", start);
      if (comma == std::string::npos)
        comma = values.length();
      try {
        result[i] = std::stof(values.substr(start, comma - start));
      } catch (...) {
        result[i] = defaultValue[i];
      }
      start = comma + 1;
    }
    return result;
  };

  auto parseString = [&displaySection](const std::string &key,
                                       const std::string &defaultValue) {
    size_t pos = displaySection.find(key + ":");
    if (pos == std::string::npos)
      return defaultValue;
    size_t valueStart =
        displaySection.find_first_not_of(" \t", pos + key.length() + 1);
    if (valueStart == std::string::npos)
      return defaultValue;
    size_t valueEnd = displaySection.find_first_of("\n\r", valueStart);
    if (valueEnd == std::string::npos)
      valueEnd = displaySection.length();
    std::string value =
        displaySection.substr(valueStart, valueEnd - valueStart);
    if (!value.empty() && value.front() == '"')
      value.erase(0, 1);
    if (!value.empty() && value.back() == '"')
      value.pop_back();
    return value;
  };

  // Parse display settings
  std::string themeStr = parseString("theme", "Dark");
  try {
    displayConfig_.theme = stringToTheme(themeStr);
  } catch (...) {
    displayConfig_.theme = Theme::Dark;
  }

  displayConfig_.backgroundColor =
      parseVec4("backgroundColor", displayConfig_.backgroundColor);
  displayConfig_.foregroundColor =
      parseVec4("foregroundColor", displayConfig_.foregroundColor);
  displayConfig_.accentColor =
      parseVec4("accentColor", displayConfig_.accentColor);
  displayConfig_.selectionColor =
      parseVec4("selectionColor", displayConfig_.selectionColor);
  displayConfig_.axisXColor =
      parseVec4("axisXColor", displayConfig_.axisXColor);
  displayConfig_.axisYColor =
      parseVec4("axisYColor", displayConfig_.axisYColor);
  displayConfig_.axisZColor =
      parseVec4("axisZColor", displayConfig_.axisZColor);

  displayConfig_.fontName = parseString("fontName", "Default");
  displayConfig_.fontSize = parseInt("fontSize", displayConfig_.fontSize);

  std::string uiSizeStr = parseString("uiSize", "Medium");
  try {
    displayConfig_.uiSize = stringToUISize(uiSizeStr);
  } catch (...) {
    displayConfig_.uiSize = UISize::Medium;
  }
}

std::string SettingsManager::SerializeGridConfig() const {
  std::stringstream ss;

  ss << "  showGrid: " << (gridConfig_.showGrid ? "true" : "false") << "\n";
  ss << "  showMinorGridLines: "
     << (gridConfig_.showMinorGridLines ? "true" : "false") << "\n";
  ss << "  majorGridSpacing: " << gridConfig_.majorGridSpacing << "\n";
  ss << "  minorDivisions: " << gridConfig_.minorDivisions << "\n";
  ss << "  majorGridColor: [" << gridConfig_.majorGridColor.x << ", "
     << gridConfig_.majorGridColor.y << ", " << gridConfig_.majorGridColor.z
     << ", " << gridConfig_.majorGridColor.w << "]\n";
  ss << "  minorGridColor: [" << gridConfig_.minorGridColor.x << ", "
     << gridConfig_.minorGridColor.y << ", " << gridConfig_.minorGridColor.z
     << ", " << gridConfig_.minorGridColor.w << "]\n";
  ss << "  gridOpacity: " << gridConfig_.gridOpacity << "\n";
  ss << "  gridType: \"" << gridTypeToString(gridConfig_.gridType) << "\"\n";

  return ss.str();
}

std::string SettingsManager::SerializeDisplayConfig() const {
  std::stringstream ss;

  ss << "  theme: \"" << themeToString(displayConfig_.theme) << "\"\n";
  ss << "  backgroundColor: [" << displayConfig_.backgroundColor.x << ", "
     << displayConfig_.backgroundColor.y << ", "
     << displayConfig_.backgroundColor.z << ", "
     << displayConfig_.backgroundColor.w << "]\n";
  ss << "  foregroundColor: [" << displayConfig_.foregroundColor.x << ", "
     << displayConfig_.foregroundColor.y << ", "
     << displayConfig_.foregroundColor.z << ", "
     << displayConfig_.foregroundColor.w << "]\n";
  ss << "  accentColor: [" << displayConfig_.accentColor.x << ", "
     << displayConfig_.accentColor.y << ", " << displayConfig_.accentColor.z
     << ", " << displayConfig_.accentColor.w << "]\n";
  ss << "  selectionColor: [" << displayConfig_.selectionColor.x << ", "
     << displayConfig_.selectionColor.y << ", "
     << displayConfig_.selectionColor.z << ", "
     << displayConfig_.selectionColor.w << "]\n";
  ss << "  axisXColor: [" << displayConfig_.axisXColor.x << ", "
     << displayConfig_.axisXColor.y << ", " << displayConfig_.axisXColor.z
     << ", " << displayConfig_.axisXColor.w << "]\n";
  ss << "  axisYColor: [" << displayConfig_.axisYColor.x << ", "
     << displayConfig_.axisYColor.y << ", " << displayConfig_.axisYColor.z
     << ", " << displayConfig_.axisYColor.w << "]\n";
  ss << "  axisZColor: [" << displayConfig_.axisZColor.x << ", "
     << displayConfig_.axisZColor.y << ", " << displayConfig_.axisZColor.z
     << ", " << displayConfig_.axisZColor.w << "]\n";
  ss << "  fontName: \"" << displayConfig_.fontName << "\"\n";
  ss << "  fontSize: " << displayConfig_.fontSize << "\n";
  ss << "  uiSize: \"" << uiSizeToString(displayConfig_.uiSize) << "\"\n";

  return ss.str();
}

} // namespace settings
} // namespace view
