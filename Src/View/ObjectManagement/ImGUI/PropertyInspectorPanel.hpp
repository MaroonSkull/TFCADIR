/**
 * @file PropertyInspectorPanel.hpp
 * @brief Dynamic property inspector panel for selected figures
 *
 * Displays and edits properties of selected figures with three states:
 * - NoSelection: Application settings
 * - SingleSelection: Full properties (geometry, transform, appearance, layer)
 * - MultiSelection: Common properties only
 */

#ifndef TFCADIR_PROPERTY_INSPECTOR_PANEL_HPP
#define TFCADIR_PROPERTY_INSPECTOR_PANEL_HPP

#include <Model/FlatFigure.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <any>
#include <chrono>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Dynamic property inspector panel for figure properties
 *
 * Displays editable properties based on current selection:
 * - No selection: Application settings
 * - Single selection: All properties of the selected figure
 * - Multi selection: Properties common to all selected figures
 *
 * Property path format (v1.3 decision tree):
 * - Scalar: Flattened ("radius", "visible")
 * - Vector/Matrix full value: Flattened ("center", "first")
 * - Vector/Matrix component: Dot notation ("center.x", "first.y")
 */
class PropertyInspectorPanel {
public:
  /**
   * @brief Construct property inspector panel
   * @param fsmAdapter Reference to UIFSMAdapter for property updates
   * @param selectionManager Reference to selection manager for querying
   * selection
   * @param model Reference to figure model for accessing figure data
   */
  PropertyInspectorPanel(UIFSMAdapter &fsmAdapter,
                         SelectionManager &selectionManager,
                         model::FlatFigures &model);

  /**
   * @brief Render the property inspector panel
   * Should be called each frame from main GUI render loop
   */
  void render();

  /**
   * @brief Invalidate cached property data
   * Call this when selection or properties change externally
   */
  void invalidateCache();

private:
  /**
   * @brief Property definition structure
   * Contains metadata for rendering a single property
   */
  struct PropertyDef {
    std::string name; ///< Display name
    std::string path; ///< Property path for updates
    std::string type; ///< Type identifier: "float", "vec2", "vec3", "bool",
                      ///< "int", "string"
    bool isComponent =
        false; ///< True if this is a component property (e.g., "center.x")
    int componentIndex =
        -1; ///< Component index for vector properties (0=x, 1=y, 2=z)

    // Value storage (cached for rendering)
    std::any value; ///< Current value (float, vec2, vec3, bool, int, string)
  };

  /**
   * @brief Property group structure
   * Groups related properties together (e.g., Geometry, Transform)
   */
  struct PropertyGroup {
    std::string name; ///< Group name (e.g., "Geometry", "Transform")
    std::vector<PropertyDef> properties; ///< Properties in this group
  };

  /**
   * @brief Cache structure for property data
   */
  struct PropertyCache {
    std::vector<PropertyGroup> groups;
    std::chrono::steady_clock::time_point lastUpdate;
    bool isValid = false;
  };

  /**
   * @brief Selection state enumeration
   */
  enum SelectionState {
    NoSelection,     ///< No figures selected - show app settings
    SingleSelection, ///< Exactly one figure selected - show all properties
    MultiSelection ///< Multiple figures selected - show common properties only
  };

  // === Main render methods ===

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

  // === Property building ===

  /**
   * @brief Build property groups for the specified figure
   * @param figure Figure to build properties for
   * @return Vector of property groups
   */
  std::vector<PropertyGroup>
  buildPropertyGroups(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build common property groups for multiple figures
   * @param figures Vector of figures to find common properties for
   * @return Vector of property groups containing only common properties
   */
  std::vector<PropertyGroup> buildCommonPropertyGroups(
      const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Build properties for a Triangle figure
   */
  void buildTriangleProperties(std::vector<PropertyGroup> &groups,
                               std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build properties for a Quad figure
   */
  void buildQuadProperties(std::vector<PropertyGroup> &groups,
                           std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build properties for a Circle figure
   */
  void buildCircleProperties(std::vector<PropertyGroup> &groups,
                             std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build properties for an Ngon figure
   */
  void buildNgonProperties(std::vector<PropertyGroup> &groups,
                           std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build properties for a CurveBezier3 figure
   */
  void buildCurveBezier3Properties(std::vector<PropertyGroup> &groups,
                                   std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Build properties for a CurveBezier4 figure
   */
  void buildCurveBezier4Properties(std::vector<PropertyGroup> &groups,
                                   std::shared_ptr<model::IFigure> figure);

  // === Common property builders ===

  /**
   * @brief Add common transform properties (id, name, position)
   */
  void addTransformProperties(std::vector<PropertyDef> &properties,
                              std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Add common appearance properties (visible, color, scribed)
   */
  void addAppearanceProperties(std::vector<PropertyDef> &properties,
                               std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Add common layer property
   */
  void addLayerProperty(std::vector<PropertyDef> &properties,
                        std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Add vec3 property with both full and component properties
   * @param properties Property vector to add to
   * @param name Display name
   * @param path Property path (full value)
   * @param value Current value
   */
  void addVec3Property(std::vector<PropertyDef> &properties,
                       const std::string &name, const std::string &path,
                       const glm::vec3 &value);

  /**
   * @brief Add vec2 property with both full and component properties
   * @param properties Property vector to add to
   * @param name Display name
   * @param path Property path (full value)
   * @param value Current value
   */
  void addVec2Property(std::vector<PropertyDef> &properties,
                       const std::string &name, const std::string &path,
                       const glm::vec2 &value);

  // === Property filtering ===

  /**
   * @brief Check if two property definitions are equivalent (same path and
   * type)
   */
  bool arePropertiesEquivalent(const PropertyDef &a,
                               const PropertyDef &b) const;

  /**
   * @brief Filter property groups to include only common properties
   * @param groups1 Property groups from first figure
   * @param groups2 Property groups from second figure
   * @return Filtered property groups with common properties only
   */
  std::vector<PropertyGroup>
  filterCommonProperties(const std::vector<PropertyGroup> &groups1,
                         const std::vector<PropertyGroup> &groups2);

  // === Property rendering ===

  /**
   * @brief Render a single property based on its type
   * @param prop Property definition
   * @param figureId ID of figure being edited
   * @return True if property was modified
   */
  bool renderProperty(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render float property with drag input
   */
  bool renderFloatProperty(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render vec2 property with X, Y drag inputs
   */
  bool renderVec2Property(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render vec3 property with X, Y, Z drag inputs
   */
  bool renderVec3Property(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render bool property as checkbox
   */
  bool renderBoolProperty(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render int property with drag input
   */
  bool renderIntProperty(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render string property with text input
   */
  bool renderStringProperty(const PropertyDef &prop, uint32_t figureId);

  /**
   * @brief Render color property (vec3) with color picker
   */
  bool renderColorProperty(const PropertyDef &prop, uint32_t figureId);

  // === Callbacks ===

  /**
   * @brief Handle selection changed callback
   */
  void onSelectionChanged();

  /**
   * @brief Handle property changed callback
   * @param figureId ID of figure whose property changed
   * @param path Path of changed property
   */
  void onPropertyChanged(uint32_t figureId, const std::string &path);

  /**
   * @brief Get helper function to retrieve figure type name
   */
  std::string getFigureTypeName(std::shared_ptr<model::IFigure> figure);

  /**
   * @brief Convert Point to glm::vec3
   * @param point Point structure with x, y, z fields
   * @return glm::vec3 with the same values
   */
  static glm::vec3 pointToVec3(const model::Point &point);

  // === Member variables ===

  UIFSMAdapter &fsmAdapter_;
  SelectionManager &selectionManager_;
  model::FlatFigures &model_;

  PropertyCache propertyCache_;

  // UI state
  std::string filterText_;

  // Callback registration
  bool callbacksRegistered_;

  static constexpr std::chrono::milliseconds CACHE_UPDATE_INTERVAL =
      std::chrono::milliseconds(100);
};

} // namespace view

#endif // TFCADIR_PROPERTY_INSPECTOR_PANEL_HPP
