/**
 * @file PropertiesPanel.hpp
 * @brief Enhanced context-aware properties panel following SolidWorks-inspired
 * design
 *
 * Phase 9.5: Properties Panel Enhancements
 *
 * Displays different properties based on selection state:
 * - Empty state: Application settings
 * - Single selection: Full object properties (geometry, appearance, layer,
 * info)
 * - Multiple selection: Common properties with mixed value indicators
 *
 * Property Categories:
 * - SELECTION: Object count, type info
 * - GEOMETRY: Type-specific properties (position, size, radius, etc.)
 * - APPEARANCE: Stroke color, fill color, line width
 * - MATERIAL: 3D-only (color, metalness, roughness)
 * - TRANSFORM: Position, rotation, scale
 * - LAYER: Layer assignment, visibility, lock
 * - INFO: ID, name, created date
 */

#ifndef TFCADIR_PROPERTIES_PANEL_HPP
#define TFCADIR_PROPERTIES_PANEL_HPP

#include <Model/FlatFigure.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/OpenGL/ImGUI/ApplicationSettings.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Enhanced context-aware properties panel for figure properties
 *
 * This panel provides a SolidWorks-inspired properties interface with:
 * - Dynamic sections based on figure type
 * - Read-only calculated properties (area, circumference, etc.)
 * - Context-aware display based on selection state
 * - Support for multiple selection with common properties
 */
class PropertiesPanel {
public:
  /**
   * @brief Construct properties panel
   * @param fsmAdapter Reference to UIFSMAdapter for property updates
   * @param selectionManager Reference to selection manager for querying
   * selection
   * @param model Reference to figure model for accessing figure data
   */
  PropertiesPanel(UIFSMAdapter &fsmAdapter, SelectionManager &selectionManager,
                  model::FlatFigures &model);

  /**
   * @brief Render the properties panel
   * Should be called each frame from main GUI render loop
   */
  void render();

  /**
   * @brief Invalidate cached property data
   * Call this when selection or properties change externally
   */
  void invalidateCache();

private:
  // === Selection State ===

  /**
   * @brief Selection state enumeration
   */
  enum class SelectionState {
    NoSelection,     ///< No figures selected - show app settings
    SingleSelection, ///< Exactly one figure selected - show all properties
    MultiSelection ///< Multiple figures selected - show common properties only
  };

  // === Property Data Structures ===

  /**
   * @brief Property value with mixed-state support for multi-selection
   */
  struct PropertyValue {
    enum class State { Normal, Mixed, ReadOnly };
    State state = State::Normal;
    float floatValue = 0.0f;
    glm::vec3 vec3Value{};
    glm::vec2 vec2Value{};
    bool boolValue = false;
    int intValue = 0;
    std::string stringValue;
  };

  /**
   * @brief Single property definition for rendering
   */
  struct Property {
    std::string label;       ///< Display label
    std::string path;        ///< Property path for updates
    std::string type;        ///< Type: "float", "vec2", "vec3", "bool", "int",
                             ///< "string", "color"
    PropertyValue value;     ///< Current value with state
    bool isReadOnly = false; ///< True for calculated properties (area, etc.)
    int componentIndex =
        -1; ///< Component index for vector properties (-1 for full vector)
  };

  /**
   * @brief Property group/section
   */
  struct PropertyGroup {
    std::string name; ///< Group name (e.g., "Geometry", "Transform")
    std::vector<Property> properties;
    bool defaultOpen = true; ///< Whether group is expanded by default
  };

  /**
   * @brief Cache structure for property data
   */
  struct PropertyCache {
    std::vector<PropertyGroup> groups;
    std::chrono::steady_clock::time_point lastUpdate;
    bool isValid = false;
    SelectionState lastState = SelectionState::NoSelection;
    size_t selectionCount = 0;
  };

  // === Member Variables ===

  UIFSMAdapter &fsmAdapter_;
  SelectionManager &selectionManager_;
  model::FlatFigures &model_;

  /// Application settings for grid/snap persistence
  ApplicationSettings appSettings_;

  PropertyCache propertyCache_;

  // UI state
  char filterBuffer_[256] = ""; ///< Filter text buffer
  bool showReadOnly_ = true;    ///< Show read-only calculated properties

  // Callback registration flag
  bool callbacksRegistered_ = false;

  // Cache update interval
  static constexpr std::chrono::milliseconds CACHE_UPDATE_INTERVAL =
      std::chrono::milliseconds(100);

  // === Main Render Methods ===

  /**
   * @brief Render panel based on current selection state
   */
  void renderByState();

  /**
   * @brief Render application settings (no selection state)
   */
  void renderAppSettings();

  /**
   * @brief Render figure properties (single/multi selection)
   * @param state Current selection state
   */
  void renderFigureProperties(SelectionState state);

  // === Property Group Renderers ===

  /**
   * @brief Render SELECTION group showing selection info
   * @param state Current selection state
   */
  void renderSelectionGroup(SelectionState state);

  /**
   * @brief Render TRANSFORM group with position, rotation, scale
   * @param figure Figure to get transform from
   */
  void renderTransformGroup(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Render GEOMETRY group with type-specific properties
   * @param figure Figure to get geometry from
   */
  void renderGeometryGroup(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Render APPEARANCE group with stroke/fill colors
   * @param figure Figure to get appearance from
   */
  void renderAppearanceGroup(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Render MATERIAL group (3D-only properties)
   * @param figure Figure to get material from
   */
  void renderMaterialGroup(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Render LAYER group with layer assignment
   * @param figure Figure to get layer info from
   */
  void renderLayerGroup(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Render INFO group with ID, name, created date
   * @param figure Figure to get info from
   */
  void renderInfoGroup(std::shared_ptr<model::IFigure> figure);

  // === Type-Specific Geometry Renderers ===

  /**
   * @brief Render Circle-specific geometry properties
   */
  void
  renderCircleGeometry(std::shared_ptr<model::Figure<model::Circle>> circle);

  /**
   * @brief Render Triangle-specific geometry properties
   */
  void renderTriangleGeometry(
      std::shared_ptr<model::Figure<model::Triangle>> triangle);

  /**
   * @brief Render Quad-specific geometry properties
   */
  void renderQuadGeometry(std::shared_ptr<model::Figure<model::Quad>> quad);

  /**
   * @brief Render Ngon-specific geometry properties
   */
  void renderNgonGeometry(std::shared_ptr<model::Figure<model::Ngon>> ngon);

  /**
   * @brief Render CurveBezier3-specific geometry properties
   */
  void renderCurveBezier3Geometry(
      std::shared_ptr<model::Figure<model::CurveBezier3>> curve);

  /**
   * @brief Render CurveBezier4-specific geometry properties
   */
  void renderCurveBezier4Geometry(
      std::shared_ptr<model::Figure<model::CurveBezier4>> curve);

  // === Property Rendering Helpers ===

  /**
   * @brief Render a float property with drag input
   * @param label Display label
   * @param value Current value
   * @param path Property path for updates
   * @param figureId Figure ID for updates
   * @param isReadOnly Whether property is read-only
   * @param speed Drag speed (default 0.1f)
   * @return True if value was modified
   */
  bool renderFloatProperty(const std::string &label, float &value,
                           const std::string &path, uint32_t figureId,
                           bool isReadOnly = false, float speed = 0.1f);

  /**
   * @brief Render a vec2 property with X, Y inputs
   */
  bool renderVec2Property(const std::string &label, glm::vec2 &value,
                          const std::string &path, uint32_t figureId,
                          bool isReadOnly = false);

  /**
   * @brief Render a vec3 property with X, Y, Z inputs
   */
  bool renderVec3Property(const std::string &label, glm::vec3 &value,
                          const std::string &path, uint32_t figureId,
                          bool isReadOnly = false);

  /**
   * @brief Render a color property with color picker
   */
  bool renderColorProperty(const std::string &label, glm::vec3 &value,
                           const std::string &path, uint32_t figureId,
                           bool isReadOnly = false);

  /**
   * @brief Render a boolean property as checkbox
   */
  bool renderBoolProperty(const std::string &label, bool &value,
                          const std::string &path, uint32_t figureId,
                          bool isReadOnly = false);

  /**
   * @brief Render an integer property with drag input
   */
  bool renderIntProperty(const std::string &label, int &value,
                         const std::string &path, uint32_t figureId,
                         bool isReadOnly = false);

  /**
   * @brief Render a string property with text input
   */
  bool renderStringProperty(const std::string &label, std::string &value,
                            const std::string &path, uint32_t figureId,
                            bool isReadOnly = false);

  /**
   * @brief Render a read-only calculated value
   * @param label Display label
   * @param value The value to display
   * @param format Printf-style format string
   */
  void renderReadOnlyValue(const std::string &label, float value,
                           const char *format = "%.2f");

  // === Multi-Selection Support ===

  /**
   * @brief Build common property groups for multiple figures
   * @param figures Vector of figures to find common properties for
   * @return Vector of property groups containing only common properties
   */
  std::vector<PropertyGroup> buildCommonPropertyGroups(
      const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Render multi-selection header with object list
   * @param figures Vector of selected figures
   */
  void renderMultiSelectionHeader(
      const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Render multi-selection action buttons (Group, Align, Distribute,
   * Delete)
   * @param selectedIds Vector of selected figure IDs
   */
  void renderMultiSelectionActions(const std::vector<uint32_t> &selectedIds);

  // === Utility Functions ===

  /**
   * @brief Get figure type name as string
   * @param figure Figure to get type name for
   * @return Type name string (e.g., "Circle", "Triangle")
   */
  std::string getFigureTypeName(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Convert Point to glm::vec3
   */
  static glm::vec3 pointToVec3(const model::Point &point);

  /**
   * @brief Calculate circle circumference
   */
  static float calculateCircumference(float radius);

  /**
   * @brief Calculate circle area
   */
  static float calculateCircleArea(float radius);

  /**
   * @brief Calculate triangle area from three vertices
   */
  static float calculateTriangleArea(const glm::vec3 &v1, const glm::vec3 &v2,
                                     const glm::vec3 &v3);

  /**
   * @brief Calculate line length between two points
   */
  static float calculateLineLength(const glm::vec3 &start,
                                   const glm::vec3 &end);

  /**
   * @brief Calculate line angle in degrees
   */
  static float calculateLineAngle(const glm::vec3 &start, const glm::vec3 &end);

  /**
   * @brief Calculate rectangle area
   */
  static float calculateRectangleArea(float width, float height);

  /**
   * @brief Calculate rectangle perimeter
   */
  static float calculateRectanglePerimeter(float width, float height);

  /**
   * @brief Calculate regular polygon area
   */
  static float calculateNgonArea(float radius, int sides);

  // === Callbacks ===

  /**
   * @brief Handle selection changed callback
   */
  void onSelectionChanged();

  /**
   * @brief Handle property changed callback
   */
  void onPropertyChanged(uint32_t figureId, const std::string &path);
};

} // namespace view

#endif // TFCADIR_PROPERTIES_PANEL_HPP
