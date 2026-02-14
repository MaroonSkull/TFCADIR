/**
 * @file SettingsConfig.cpp
 * @brief Implementation of settings configuration structures
 */

#include "SettingsConfig.hpp"

#include <stdexcept>

namespace view {
namespace settings {

bool GridConfig::operator==(const GridConfig &other) const {
  return showGrid == other.showGrid &&
         showMinorGridLines == other.showMinorGridLines &&
         majorGridSpacing == other.majorGridSpacing &&
         minorDivisions == other.minorDivisions &&
         majorGridColor == other.majorGridColor &&
         minorGridColor == other.minorGridColor &&
         gridOpacity == other.gridOpacity && gridType == other.gridType;
}

bool GridConfig::operator!=(const GridConfig &other) const {
  return !(*this == other);
}

bool DisplayConfig::operator==(const DisplayConfig &other) const {
  return theme == other.theme && backgroundColor == other.backgroundColor &&
         foregroundColor == other.foregroundColor &&
         accentColor == other.accentColor &&
         selectionColor == other.selectionColor &&
         axisXColor == other.axisXColor && axisYColor == other.axisYColor &&
         axisZColor == other.axisZColor && fontName == other.fontName &&
         fontSize == other.fontSize && uiSize == other.uiSize;
}

bool DisplayConfig::operator!=(const DisplayConfig &other) const {
  return !(*this == other);
}

bool AppSettings::operator==(const AppSettings &other) const {
  return gridConfig == other.gridConfig && displayConfig == other.displayConfig;
}

bool AppSettings::operator!=(const AppSettings &other) const {
  return !(*this == other);
}

std::string gridTypeToString(GridType type) {
  switch (type) {
  case GridType::Cartesian:
    return "Cartesian";
  case GridType::Isometric:
    return "Isometric";
  case GridType::Polar:
    return "Polar";
  default:
    return "Cartesian";
  }
}

GridType stringToGridType(const std::string &str) {
  if (str == "Cartesian") {
    return GridType::Cartesian;
  }
  if (str == "Isometric") {
    return GridType::Isometric;
  }
  if (str == "Polar") {
    return GridType::Polar;
  }
  throw std::invalid_argument("Invalid grid type: " + str);
}

std::string themeToString(Theme theme) {
  switch (theme) {
  case Theme::Dark:
    return "Dark";
  case Theme::Light:
    return "Light";
  case Theme::Custom:
    return "Custom";
  default:
    return "Dark";
  }
}

Theme stringToTheme(const std::string &str) {
  if (str == "Dark") {
    return Theme::Dark;
  }
  if (str == "Light") {
    return Theme::Light;
  }
  if (str == "Custom") {
    return Theme::Custom;
  }
  throw std::invalid_argument("Invalid theme: " + str);
}

std::string uiSizeToString(UISize size) {
  switch (size) {
  case UISize::Small:
    return "Small";
  case UISize::Medium:
    return "Medium";
  case UISize::Large:
    return "Large";
  default:
    return "Medium";
  }
}

UISize stringToUISize(const std::string &str) {
  if (str == "Small") {
    return UISize::Small;
  }
  if (str == "Medium") {
    return UISize::Medium;
  }
  if (str == "Large") {
    return UISize::Large;
  }
  throw std::invalid_argument("Invalid UI size: " + str);
}

} // namespace settings
} // namespace view
