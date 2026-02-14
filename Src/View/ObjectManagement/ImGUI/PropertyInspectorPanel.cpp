/**
 * @file PropertyInspectorPanel.cpp
 * @brief Dynamic property inspector panel implementation
 */

#include <View/ObjectManagement/ImGUI/PropertyInspectorPanel.hpp>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace view {

// === Constructor ===

PropertyInspectorPanel::PropertyInspectorPanel(
    UIFSMAdapter &fsmAdapter, SelectionManager &selectionManager,
    model::FlatFigures &model)
    : fsmAdapter_(fsmAdapter), selectionManager_(selectionManager),
      model_(model), callbacksRegistered_(false) {

  // Register callbacks for selection and property changes
  fsmAdapter_.setSelectionChangedCallback(
      [this](const std::vector<uint32_t> &ids) { this->onSelectionChanged(); });
  fsmAdapter_.setPropertyChangedCallback(
      [this](uint32_t id, const std::string &path) {
        this->onPropertyChanged(id, path);
      });

  callbacksRegistered_ = true;
}

// === Public render methods ===

void PropertyInspectorPanel::render() {
  // Cache update with rate limiting
  auto now = std::chrono::steady_clock::now();
  if (!propertyCache_.isValid ||
      (now - propertyCache_.lastUpdate) > CACHE_UPDATE_INTERVAL) {
    propertyCache_.isValid = false;
  }

  ImGui::Begin("Properties");

  // Add filter text box
  char filterBuffer[256];
  std::strncpy(filterBuffer, filterText_.c_str(), sizeof(filterBuffer));
  filterBuffer[sizeof(filterBuffer) - 1] = '\0';
  if (ImGui::InputText("##filter", filterBuffer, sizeof(filterBuffer))) {
    filterText_ = filterBuffer;
    propertyCache_.isValid = false;
  }
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Filter");

  ImGui::Separator();

  renderByState();

  ImGui::End();
}

void PropertyInspectorPanel::invalidateCache() {
  propertyCache_.isValid = false;
}

// === Main render methods ===

void PropertyInspectorPanel::renderByState() {
  auto selectedIds = selectionManager_.getSelectedFigureIds();
  SelectionState state;

  if (selectedIds.empty()) {
    state = NoSelection;
  } else if (selectedIds.size() == 1) {
    state = SingleSelection;
  } else {
    state = MultiSelection;
  }

  renderFigureProperties(state);
}

void PropertyInspectorPanel::renderAppSettings() {
  // No selection - show application settings
  // For now, show a placeholder message
  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No selection");
  ImGui::Text("Application settings will be displayed here.");
}

void PropertyInspectorPanel::renderFigureProperties(SelectionState state) {
  auto selectedIds = selectionManager_.getSelectedFigureIds();

  if (state == NoSelection) {
    renderAppSettings();
    return;
  }

  // Build or update cache
  if (!propertyCache_.isValid) {
    std::vector<std::shared_ptr<model::IFigure>> selectedFigures;
    for (uint32_t id : selectedIds) {
      auto figure = model_.getFigure(id);
      if (figure) {
        selectedFigures.push_back(figure);
      }
    }

    if (selectedFigures.empty()) {
      renderAppSettings();
      return;
    }

    if (state == SingleSelection) {
      propertyCache_.groups = buildPropertyGroups(selectedFigures[0]);
    } else {
      propertyCache_.groups = buildCommonPropertyGroups(selectedFigures);
    }

    propertyCache_.lastUpdate = std::chrono::steady_clock::now();
    propertyCache_.isValid = true;
  }

  // Get primary selection for property updates
  uint32_t primaryId = selectionManager_.getPrimarySelectionId();

  // Show selection header
  if (state == MultiSelection) {
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%zu figures selected",
                       selectedIds.size());
    ImGui::Text("Editing common properties of %s",
                getFigureTypeName(model_.getFigure(primaryId)).c_str());
    ImGui::Separator();
  } else {
    auto primaryFigure = model_.getFigure(primaryId);
    if (primaryFigure) {
      ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                         getFigureTypeName(primaryFigure).c_str());
      ImGui::Text("ID: %u", primaryId);
      ImGui::Separator();
    }
  }

  // Render property groups
  for (const auto &group : propertyCache_.groups) {
    // Apply filter
    if (!filterText_.empty()) {
      bool groupMatches = false;
      for (const auto &prop : group.properties) {
        if (prop.name.find(filterText_) != std::string::npos) {
          groupMatches = true;
          break;
        }
      }
      if (!groupMatches) {
        continue;
      }
    }

    // Render group
    if (ImGui::CollapsingHeader(group.name.c_str(),
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      for (const auto &prop : group.properties) {
        // Skip filtered properties
        if (!filterText_.empty() &&
            prop.name.find(filterText_) == std::string::npos) {
          continue;
        }

        ImGui::PushID(prop.path.c_str());
        renderProperty(prop, primaryId);
        ImGui::PopID();
      }
    }
  }
}

// === Property building ===

std::vector<PropertyInspectorPanel::PropertyGroup>
PropertyInspectorPanel::buildPropertyGroups(
    std::shared_ptr<model::IFigure> figure) {
  std::vector<PropertyGroup> groups;

  if (auto triangle =
          std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    buildTriangleProperties(groups, figure);
  } else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
    buildQuadProperties(groups, figure);
  } else if (auto circle =
                 std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                     figure)) {
    buildCircleProperties(groups, figure);
  } else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
    buildNgonProperties(groups, figure);
  } else if (auto curve3 =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
    buildCurveBezier3Properties(groups, figure);
  } else if (auto curve4 =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
    buildCurveBezier4Properties(groups, figure);
  }

  return groups;
}

std::vector<PropertyInspectorPanel::PropertyGroup>
PropertyInspectorPanel::buildCommonPropertyGroups(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {

  if (figures.empty()) {
    return {};
  }

  // Start with properties of first figure
  auto commonGroups = buildPropertyGroups(figures[0]);

  // Intersect with properties of other figures
  for (size_t i = 1; i < figures.size(); ++i) {
    auto figureGroups = buildPropertyGroups(figures[i]);
    commonGroups = filterCommonProperties(commonGroups, figureGroups);
  }

  return commonGroups;
}

void PropertyInspectorPanel::buildTriangleProperties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto triangle =
      std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure);
  if (!triangle) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Vertices as vec3 properties (Point has x, y, z directly)
  addVec3Property(geometryGroup.properties, "First Vertex", "first",
                  pointToVec3(triangle->first));
  addVec3Property(geometryGroup.properties, "Second Vertex", "second",
                  pointToVec3(triangle->second));
  addVec3Property(geometryGroup.properties, "Third Vertex", "third",
                  pointToVec3(triangle->third));

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

void PropertyInspectorPanel::buildQuadProperties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure);
  if (!quad) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Vertices as vec3 properties
  addVec3Property(geometryGroup.properties, "First Vertex", "first",
                  pointToVec3(quad->first));
  addVec3Property(geometryGroup.properties, "Second Vertex", "second",
                  pointToVec3(quad->second));
  addVec3Property(geometryGroup.properties, "Third Vertex", "third",
                  pointToVec3(quad->third));
  addVec3Property(geometryGroup.properties, "Fourth Vertex", "fourth",
                  pointToVec3(quad->fourth));

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

void PropertyInspectorPanel::buildCircleProperties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto circle = std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure);
  if (!circle) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Radius property
  geometryGroup.properties.push_back(
      {"Radius", "radius", "float", false, -1, circle->radius});

  // Center as vec3 property (v1.3 FIX: include explicit center.z)
  addVec3Property(geometryGroup.properties, "Center", "center",
                  pointToVec3(circle->center));

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

void PropertyInspectorPanel::buildNgonProperties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure);
  if (!ngon) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Center property
  addVec3Property(geometryGroup.properties, "Center", "center",
                  pointToVec3(ngon->center));

  // Radius property
  geometryGroup.properties.push_back(
      {"Radius", "radius", "float", false, -1, ngon->radius});

  // Number of sides property (n is float in Ngon struct)
  geometryGroup.properties.push_back(
      {"Sides", "n", "float", false, -1, ngon->n});

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

void PropertyInspectorPanel::buildCurveBezier3Properties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto curve =
      std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure);
  if (!curve) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Control points as vec3 properties (CurveBezier3 has start, end, first)
  addVec3Property(geometryGroup.properties, "Start Point", "start",
                  pointToVec3(curve->start));
  addVec3Property(geometryGroup.properties, "End Point", "end",
                  pointToVec3(curve->end));
  addVec3Property(geometryGroup.properties, "Control Point", "first",
                  pointToVec3(curve->first));

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

void PropertyInspectorPanel::buildCurveBezier4Properties(
    std::vector<PropertyGroup> &groups,
    std::shared_ptr<model::IFigure> figure) {
  auto curve =
      std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure);
  if (!curve) {
    return;
  }

  // Geometry group
  PropertyGroup geometryGroup;
  geometryGroup.name = "Geometry";

  // Control points as vec3 properties (CurveBezier4 has start, end, first,
  // second)
  addVec3Property(geometryGroup.properties, "Start Point", "start",
                  pointToVec3(curve->start));
  addVec3Property(geometryGroup.properties, "End Point", "end",
                  pointToVec3(curve->end));
  addVec3Property(geometryGroup.properties, "Control Point 1", "first",
                  pointToVec3(curve->first));
  addVec3Property(geometryGroup.properties, "Control Point 2", "second",
                  pointToVec3(curve->second));

  groups.push_back(geometryGroup);

  // Transform group
  PropertyGroup transformGroup;
  transformGroup.name = "Transform";
  addTransformProperties(transformGroup.properties, figure);
  groups.push_back(transformGroup);

  // Appearance group
  PropertyGroup appearanceGroup;
  appearanceGroup.name = "Appearance";
  addAppearanceProperties(appearanceGroup.properties, figure);
  groups.push_back(appearanceGroup);
}

// === Common property builders ===

void PropertyInspectorPanel::addTransformProperties(
    std::vector<PropertyDef> &properties,
    std::shared_ptr<model::IFigure> figure) {
  // ID (read-only)
  properties.push_back(
      {"ID", "id", "int", false, -1, static_cast<int>(figure->getId())});

  // Name
  properties.push_back(
      {"Name", "name", "string", false, -1, figure->getName()});

  // Position
  addVec3Property(properties, "Position", "position", figure->getPosition());
}

void PropertyInspectorPanel::addAppearanceProperties(
    std::vector<PropertyDef> &properties,
    std::shared_ptr<model::IFigure> figure) {
  // Scribed state
  auto figureTyped =
      std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure);
  if (figureTyped) {
    bool scribed = (figureTyped->scribed_ != model::isScribed::no);
    properties.push_back({"Scribed", "scribed", "bool", false, -1, scribed});
  }
}

void PropertyInspectorPanel::addLayerProperty(
    std::vector<PropertyDef> &properties,
    std::shared_ptr<model::IFigure> figure) {
  // Layer property - not currently available in Figure template
  // Placeholder for future implementation
}

void PropertyInspectorPanel::addVec3Property(
    std::vector<PropertyDef> &properties, const std::string &name,
    const std::string &path, const glm::vec3 &value) {
  // Full value property (flattened path as per v1.3 decision tree)
  PropertyDef fullProp;
  fullProp.name = name;
  fullProp.path = path;
  fullProp.type = "vec3";
  fullProp.isComponent = false;
  fullProp.componentIndex = -1;
  fullProp.value = value;
  properties.push_back(fullProp);

  // Component properties (dot notation for individual components)
  const char *components[] = {"X", "Y", "Z"};
  for (int i = 0; i < 3; ++i) {
    PropertyDef compProp;
    compProp.name = std::string(name) + " " + components[i];
    compProp.path = path + "." + std::string(1, 'x' + i);
    compProp.type = "float";
    compProp.isComponent = true;
    compProp.componentIndex = i;
    compProp.value = value[i];
    properties.push_back(compProp);
  }
}

void PropertyInspectorPanel::addVec2Property(
    std::vector<PropertyDef> &properties, const std::string &name,
    const std::string &path, const glm::vec2 &value) {
  // Full value property
  PropertyDef fullProp;
  fullProp.name = name;
  fullProp.path = path;
  fullProp.type = "vec2";
  fullProp.isComponent = false;
  fullProp.componentIndex = -1;
  fullProp.value = value;
  properties.push_back(fullProp);

  // Component properties
  const char *components[] = {"X", "Y"};
  for (int i = 0; i < 2; ++i) {
    PropertyDef compProp;
    compProp.name = std::string(name) + " " + components[i];
    compProp.path = path + "." + std::string(1, 'x' + i);
    compProp.type = "float";
    compProp.isComponent = true;
    compProp.componentIndex = i;
    compProp.value = value[i];
    properties.push_back(compProp);
  }
}

// === Property filtering ===

bool PropertyInspectorPanel::arePropertiesEquivalent(
    const PropertyDef &a, const PropertyDef &b) const {
  return a.path == b.path && a.type == b.type;
}

std::vector<PropertyInspectorPanel::PropertyGroup>
PropertyInspectorPanel::filterCommonProperties(
    const std::vector<PropertyGroup> &groups1,
    const std::vector<PropertyGroup> &groups2) {

  std::vector<PropertyGroup> commonGroups;

  for (const auto &group1 : groups1) {
    // Find matching group in groups2
    auto it = std::find_if(
        groups2.begin(), groups2.end(),
        [&group1](const PropertyGroup &g) { return g.name == group1.name; });

    if (it == groups2.end()) {
      continue;
    }

    // Find common properties
    PropertyGroup commonGroup;
    commonGroup.name = group1.name;

    for (const auto &prop1 : group1.properties) {
      auto propIt = std::find_if(it->properties.begin(), it->properties.end(),
                                 [&prop1, this](const PropertyDef &p) {
                                   return arePropertiesEquivalent(prop1, p);
                                 });

      if (propIt != it->properties.end()) {
        commonGroup.properties.push_back(prop1);
      }
    }

    if (!commonGroup.properties.empty()) {
      commonGroups.push_back(commonGroup);
    }
  }

  return commonGroups;
}

// === Property rendering ===

bool PropertyInspectorPanel::renderProperty(const PropertyDef &prop,
                                            uint32_t figureId) {
  if (prop.type == "float") {
    return renderFloatProperty(prop, figureId);
  } else if (prop.type == "vec2") {
    return renderVec2Property(prop, figureId);
  } else if (prop.type == "vec3") {
    return renderVec3Property(prop, figureId);
  } else if (prop.type == "bool") {
    return renderBoolProperty(prop, figureId);
  } else if (prop.type == "int") {
    return renderIntProperty(prop, figureId);
  } else if (prop.type == "string") {
    return renderStringProperty(prop, figureId);
  } else if (prop.type == "color") {
    return renderColorProperty(prop, figureId);
  }
  return false;
}

bool PropertyInspectorPanel::renderFloatProperty(const PropertyDef &prop,
                                                 uint32_t figureId) {
  float value = std::any_cast<float>(prop.value);
  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(150);

  if (ImGui::DragFloat("##value", &value, 0.1f)) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, value);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderVec2Property(const PropertyDef &prop,
                                                uint32_t figureId) {
  glm::vec2 value = std::any_cast<glm::vec2>(prop.value);
  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(200);

  if (ImGui::DragFloat2("##value", glm::value_ptr(value), 0.1f)) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, value);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderVec3Property(const PropertyDef &prop,
                                                uint32_t figureId) {
  glm::vec3 value = std::any_cast<glm::vec3>(prop.value);
  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(200);

  if (ImGui::DragFloat3("##value", glm::value_ptr(value), 0.1f)) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, value);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderBoolProperty(const PropertyDef &prop,
                                                uint32_t figureId) {
  bool value = std::any_cast<bool>(prop.value);
  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();

  if (ImGui::Checkbox("##value", &value)) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, value);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderIntProperty(const PropertyDef &prop,
                                               uint32_t figureId) {
  int value = std::any_cast<int>(prop.value);
  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(150);

  if (ImGui::DragInt("##value", &value)) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, value);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderStringProperty(const PropertyDef &prop,
                                                  uint32_t figureId) {
  std::string value = std::any_cast<std::string>(prop.value);
  char buffer[256];
  std::strncpy(buffer, value.c_str(), sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0';

  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(200);

  if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
    fsmAdapter_.updateFigureProperty(figureId, prop.path, std::string(buffer));
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

bool PropertyInspectorPanel::renderColorProperty(const PropertyDef &prop,
                                                 uint32_t figureId) {
  glm::vec3 value = std::any_cast<glm::vec3>(prop.value);
  ImVec4 colorValue(value.x, value.y, value.z, 1.0f);

  ImGui::Text("%s", prop.name.c_str());
  ImGui::SameLine();

  if (ImGui::ColorEdit3("##value", &colorValue.x)) {
    glm::vec3 newColor(colorValue.x, colorValue.y, colorValue.z);
    fsmAdapter_.updateFigureProperty(figureId, prop.path, newColor);
    propertyCache_.isValid = false;
    return true;
  }
  return false;
}

// === Callbacks ===

void PropertyInspectorPanel::onSelectionChanged() {
  propertyCache_.isValid = false;
}

void PropertyInspectorPanel::onPropertyChanged(uint32_t figureId,
                                               const std::string &path) {
  propertyCache_.isValid = false;
}

// === Helper functions ===

std::string PropertyInspectorPanel::getFigureTypeName(
    std::shared_ptr<model::IFigure> figure) {
  if (std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    return model::Triangle::name;
  } else if (std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
    return model::Quad::name;
  } else if (std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    return model::Circle::name;
  } else if (std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
    return model::Ngon::name;
  } else if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                 figure)) {
    return model::CurveBezier3::name;
  } else if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                 figure)) {
    return model::CurveBezier4::name;
  }
  return "Unknown";
}

/**
 * @brief Convert Point to glm::vec3
 * @param point Point structure with x, y, z fields
 * @return glm::vec3 with the same values
 */
glm::vec3 PropertyInspectorPanel::pointToVec3(const model::Point &point) {
  return glm::vec3(point.x, point.y, point.z);
}

} // namespace view
