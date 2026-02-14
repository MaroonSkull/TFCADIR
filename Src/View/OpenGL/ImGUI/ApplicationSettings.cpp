/**
 * @file ApplicationSettings.cpp
 * @brief Implementation of ApplicationSettings persistence
 *
 * Phase 9.5-r: Properties Panel Rework
 */

#include "ApplicationSettings.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace view {

ApplicationSettings::ApplicationSettings(const std::string &configPath)
    : configPath_(configPath) {
  // Try to load existing settings
  if (!load()) {
    // If loading failed, use defaults and save
    spdlog::info("Using default application settings");
    save();
  }
}

ApplicationSettings::~ApplicationSettings() { save(); }

// === Grid Settings ===

const GridSettings &ApplicationSettings::getGridSettings() const {
  return gridSettings_;
}

void ApplicationSettings::setGridSettings(const GridSettings &settings) {
  gridSettings_ = settings;
  notifySettingsChanged();
}

bool ApplicationSettings::isGridVisible() const {
  return gridSettings_.visible;
}

void ApplicationSettings::setGridVisible(bool visible) {
  gridSettings_.visible = visible;
  notifySettingsChanged();
}

float ApplicationSettings::getGridMajorSpacing() const {
  return gridSettings_.majorSpacing;
}

void ApplicationSettings::setGridMajorSpacing(float spacing) {
  gridSettings_.majorSpacing = spacing;
  notifySettingsChanged();
}

// === Snap Settings ===

const SnapSettings &ApplicationSettings::getSnapSettings() const {
  return snapSettings_;
}

void ApplicationSettings::setSnapSettings(const SnapSettings &settings) {
  snapSettings_ = settings;
  notifySettingsChanged();
}

bool ApplicationSettings::isSnapEnabled() const {
  return snapSettings_.gridEnabled;
}

void ApplicationSettings::setSnapEnabled(bool enabled) {
  snapSettings_.gridEnabled = enabled;
  notifySettingsChanged();
}

// === Persistence ===

bool ApplicationSettings::load() {
  std::ifstream file(configPath_);
  if (!file.is_open()) {
    spdlog::debug("Could not open settings file: {}", configPath_);
    return false;
  }

  std::string line;
  std::string currentSection;

  while (std::getline(file, line)) {
    // Remove leading/trailing whitespace
    size_t start = line.find_first_not_of(" \t");
    size_t end = line.find_last_not_of(" \t\r\n");

    if (start == std::string::npos || end == std::string::npos) {
      continue; // Empty line
    }

    line = line.substr(start, end - start + 1);

    // Skip comments
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Check for section headers
    if (line[0] == '[' && line.back() == ']') {
      currentSection = line.substr(1, line.length() - 2);
      continue;
    }

    // Parse key-value pairs
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, eqPos);
    std::string value = line.substr(eqPos + 1);

    // Trim whitespace from key and value
    start = key.find_first_not_of(" \t");
    end = key.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
      key = key.substr(start, end - start + 1);
    }

    start = value.find_first_not_of(" \t");
    end = value.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
      value = value.substr(start, end - start + 1);
    }

    // Parse based on section
    if (currentSection == "grid") {
      if (key == "visible") {
        gridSettings_.visible = (value == "true");
      } else if (key == "show_minor_lines") {
        gridSettings_.showMinorLines = (value == "true");
      } else if (key == "show_axes") {
        gridSettings_.showAxes = (value == "true");
      } else if (key == "show_origin") {
        gridSettings_.showOrigin = (value == "true");
      } else if (key == "major_spacing") {
        gridSettings_.majorSpacing = std::stof(value);
      } else if (key == "minor_divisions") {
        gridSettings_.minorDivisions = std::stoi(value);
      } else if (key == "opacity") {
        gridSettings_.opacity = std::stof(value);
      }
    } else if (currentSection == "snap") {
      if (key == "grid_enabled") {
        snapSettings_.gridEnabled = (value == "true");
      } else if (key == "endpoint_enabled") {
        snapSettings_.endpointEnabled = (value == "true");
      } else if (key == "midpoint_enabled") {
        snapSettings_.midpointEnabled = (value == "true");
      } else if (key == "center_enabled") {
        snapSettings_.centerEnabled = (value == "true");
      } else if (key == "intersection_enabled") {
        snapSettings_.intersectionEnabled = (value == "true");
      } else if (key == "nearest_enabled") {
        snapSettings_.nearestEnabled = (value == "true");
      } else if (key == "tangent_enabled") {
        snapSettings_.tangentEnabled = (value == "true");
      } else if (key == "perpendicular_enabled") {
        snapSettings_.perpendicularEnabled = (value == "true");
      } else if (key == "tolerance_pixels") {
        snapSettings_.tolerancePixels = std::stof(value);
      } else if (key == "show_indicators") {
        snapSettings_.showIndicators = (value == "true");
      }
    }
  }

  spdlog::info("Loaded application settings from {}", configPath_);
  return true;
}

bool ApplicationSettings::save() {
  if (!ensureConfigDirectory()) {
    spdlog::error("Could not create config directory");
    return false;
  }

  std::ofstream file(configPath_);
  if (!file.is_open()) {
    spdlog::error("Could not open settings file for writing: {}", configPath_);
    return false;
  }

  // Write header
  file << "# TFCADIR Application Settings\n";
  file << "# Auto-generated - Do not edit manually\n\n";

  // Write grid settings
  file << "[grid]\n";
  file << "visible = " << (gridSettings_.visible ? "true" : "false") << "\n";
  file << "show_minor_lines = "
       << (gridSettings_.showMinorLines ? "true" : "false") << "\n";
  file << "show_axes = " << (gridSettings_.showAxes ? "true" : "false") << "\n";
  file << "show_origin = " << (gridSettings_.showOrigin ? "true" : "false")
       << "\n";
  file << "major_spacing = " << gridSettings_.majorSpacing << "\n";
  file << "minor_divisions = " << gridSettings_.minorDivisions << "\n";
  file << "opacity = " << gridSettings_.opacity << "\n\n";

  // Write snap settings
  file << "[snap]\n";
  file << "grid_enabled = " << (snapSettings_.gridEnabled ? "true" : "false")
       << "\n";
  file << "endpoint_enabled = "
       << (snapSettings_.endpointEnabled ? "true" : "false") << "\n";
  file << "midpoint_enabled = "
       << (snapSettings_.midpointEnabled ? "true" : "false") << "\n";
  file << "center_enabled = "
       << (snapSettings_.centerEnabled ? "true" : "false") << "\n";
  file << "intersection_enabled = "
       << (snapSettings_.intersectionEnabled ? "true" : "false") << "\n";
  file << "nearest_enabled = "
       << (snapSettings_.nearestEnabled ? "true" : "false") << "\n";
  file << "tangent_enabled = "
       << (snapSettings_.tangentEnabled ? "true" : "false") << "\n";
  file << "perpendicular_enabled = "
       << (snapSettings_.perpendicularEnabled ? "true" : "false") << "\n";
  file << "tolerance_pixels = " << snapSettings_.tolerancePixels << "\n";
  file << "show_indicators = "
       << (snapSettings_.showIndicators ? "true" : "false") << "\n";

  spdlog::debug("Saved application settings to {}", configPath_);
  return true;
}

// === Callbacks ===

void ApplicationSettings::setSettingsChangedCallback(
    SettingsChangedCallback callback) {
  settingsChangedCallback_ = std::move(callback);
}

void ApplicationSettings::notifySettingsChanged() {
  if (settingsChangedCallback_) {
    settingsChangedCallback_();
  }
}

bool ApplicationSettings::ensureConfigDirectory() {
  std::filesystem::path p(configPath_);
  std::filesystem::path dir = p.parent_path();

  if (!dir.empty() && !std::filesystem::exists(dir)) {
    std::error_code ec;
    if (!std::filesystem::create_directories(dir, ec)) {
      spdlog::error("Failed to create config directory: {}", ec.message());
      return false;
    }
  }

  return true;
}

} // namespace view
