/**
 * @file PropertiesPanel.cpp
 * @brief Enhanced context-aware properties panel implementation
 *
 * Phase 9.5: Properties Panel Enhancements
 */

#include <View/OpenGL/ImGUI/PropertiesPanel.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace view {

// === Constants ===
constexpr float PI = 3.14159265358979323846f;

// === Constructor ===

PropertiesPanel::PropertiesPanel(UIFSMAdapter &fsmAdapter,
                                 SelectionManager &selectionManager,
                                 model::FlatFigures &model)
    : fsmAdapter_(fsmAdapter), selectionManager_(selectionManager),
      model_(model), callbacksRegistered_(false) {

  // Register callbacks for selection and property changes
  fsmAdapter_.setSelectionChangedCallback(
      [this](const std::vector<uint32_t> & /*ids*/) {
        this->onSelectionChanged();
      });
  fsmAdapter_.setPropertyChangedCallback(
      [this](uint32_t id, const std::string &path) {
        this->onPropertyChanged(id, path);
      });

  callbacksRegistered_ = true;
}

// === Public Methods ===

void PropertiesPanel::render() {
  // Cache update with rate limiting
  auto now = std::chrono::steady_clock::now();
  if (!propertyCache_.isValid ||
      (now - propertyCache_.lastUpdate) > CACHE_UPDATE_INTERVAL) {
    propertyCache_.isValid = false;
  }

  ImGui::Begin("Properties");

  // Header with filter
  ImGui::Text("PROPERTIES");
  ImGui::SameLine(ImGui::GetWindowWidth() - 60);
  if (ImGui::SmallButton("Filter")) {
    ImGui::OpenPopup("FilterPopup");
  }

  if (ImGui::BeginPopup("FilterPopup")) {
    ImGui::Checkbox("Show Read-Only Values", &showReadOnly_);
    ImGui::InputText("Search", filterBuffer_, sizeof(filterBuffer_));
    ImGui::EndPopup();
  }

  ImGui::Separator();

  renderByState();

  ImGui::End();
}

void PropertiesPanel::invalidateCache() { propertyCache_.isValid = false; }

// === Main Render Methods ===

void PropertiesPanel::renderByState() {
  auto selectedIds = selectionManager_.getSelectedFigureIds();
  SelectionState state;

  if (selectedIds.empty()) {
    state = SelectionState::NoSelection;
  } else if (selectedIds.size() == 1) {
    state = SelectionState::SingleSelection;
  } else {
    state = SelectionState::MultiSelection;
  }

  // Check if state changed
  if (propertyCache_.lastState != state ||
      propertyCache_.selectionCount != selectedIds.size()) {
    propertyCache_.isValid = false;
    propertyCache_.lastState = state;
    propertyCache_.selectionCount = selectedIds.size();
  }

  switch (state) {
  case SelectionState::NoSelection:
    renderAppSettings();
    break;
  case SelectionState::SingleSelection:
  case SelectionState::MultiSelection:
    renderFigureProperties(state);
    break;
  }
}

void PropertiesPanel::renderAppSettings() {
  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No Selection");
  ImGui::Spacing();
  ImGui::TextWrapped("Select an object to view its properties.");

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Application settings placeholder
  if (ImGui::CollapsingHeader("Application Settings",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Grid:");
    ImGui::Indent();
    bool gridEnabled = true;
    ImGui::Checkbox("Show Grid", &gridEnabled);
    float gridSize = 1.0f;
    ImGui::DragFloat("Grid Size", &gridSize, 0.1f, 0.1f, 10.0f);
    ImGui::Unindent();

    ImGui::Spacing();

    ImGui::Text("Snap:");
    ImGui::Indent();
    bool snapEnabled = true;
    ImGui::Checkbox("Enable Snap", &snapEnabled);
    ImGui::Unindent();
  }
}

void PropertiesPanel::renderFigureProperties(SelectionState state) {
  auto selectedIds = selectionManager_.getSelectedFigureIds();

  if (selectedIds.empty()) {
    renderAppSettings();
    return;
  }

  // Get selected figures
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

  // Render selection group
  renderSelectionGroup(state);

  // Get primary selection for property updates
  uint32_t primaryId = selectionManager_.getPrimarySelectionId();
  auto primaryFigure = model_.getFigure(primaryId);

  if (!primaryFigure) {
    return;
  }

  // Multi-selection header
  if (state == SelectionState::MultiSelection) {
    renderMultiSelectionHeader(selectedFigures);
    renderMultiSelectionActions(selectedIds);
    ImGui::Separator();
  }

  // Render property groups
  renderTransformGroup(primaryFigure);
  renderGeometryGroup(primaryFigure);
  renderAppearanceGroup(primaryFigure);
  renderMaterialGroup(primaryFigure);
  renderLayerGroup(primaryFigure);
  renderInfoGroup(primaryFigure);
}

// === Property Group Renderers ===

void PropertiesPanel::renderSelectionGroup(SelectionState state) {
  auto selectedIds = selectionManager_.getSelectedFigureIds();

  if (state == SelectionState::SingleSelection) {
    auto figure = model_.getFigure(selectedIds[0]);
    if (figure) {
      ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                         getFigureTypeName(figure).c_str());
    }
  } else if (state == SelectionState::MultiSelection) {
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%zu objects selected",
                       selectedIds.size());
  }
}

void PropertiesPanel::renderTransformGroup(
    std::shared_ptr<model::IFigure> figure) {
  if (!ImGui::CollapsingHeader("TRANSFORM", ImGuiTreeNodeFlags_DefaultOpen)) {
    return;
  }

  uint32_t figureId = figure->getId();
  glm::vec3 position = figure->getPosition();

  ImGui::Text("Position:");
  ImGui::Indent();
  renderVec3Property("Position", position, "position", figureId);
  ImGui::Unindent();

  // Rotation (placeholder - not implemented in IFigure)
  ImGui::Text("Rotation:");
  ImGui::Indent();
  float rotation = 0.0f;
  ImGui::BeginDisabled();
  ImGui::DragFloat("##rotation", &rotation, 1.0f, 0.0f, 360.0f, "%.2f°");
  ImGui::EndDisabled();
  ImGui::Unindent();

  // Scale (placeholder - not implemented in IFigure)
  ImGui::Text("Scale:");
  ImGui::Indent();
  glm::vec3 scale(1.0f);
  ImGui::BeginDisabled();
  ImGui::DragFloat3("##scale", glm::value_ptr(scale), 0.01f, 0.01f, 100.0f,
                    "%.2f");
  ImGui::EndDisabled();
  ImGui::Unindent();
}

void PropertiesPanel::renderGeometryGroup(
    std::shared_ptr<model::IFigure> figure) {
  if (!ImGui::CollapsingHeader("GEOMETRY", ImGuiTreeNodeFlags_DefaultOpen)) {
    return;
  }

  // Type-specific geometry
  if (auto circle =
          std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    renderCircleGeometry(circle);
  } else if (auto triangle =
                 std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
                     figure)) {
    renderTriangleGeometry(triangle);
  } else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
    renderQuadGeometry(quad);
  } else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
    renderNgonGeometry(ngon);
  } else if (auto curve3 =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
    renderCurveBezier3Geometry(curve3);
  } else if (auto curve4 =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
    renderCurveBezier4Geometry(curve4);
  } else {
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Unknown figure type");
  }
}

void PropertiesPanel::renderAppearanceGroup(
    std::shared_ptr<model::IFigure> figure) {
  if (!ImGui::CollapsingHeader("APPEARANCE", ImGuiTreeNodeFlags_DefaultOpen)) {
    return;
  }

  uint32_t figureId = figure->getId();

  // Stroke color (placeholder)
  glm::vec3 strokeColor(1.0f, 1.0f, 1.0f);
  ImGui::Text("Stroke Color:");
  ImGui::SameLine();
  ImGui::ColorEdit3("##strokeColor", glm::value_ptr(strokeColor),
                    ImGuiColorEditFlags_NoInputs);

  // Fill color (placeholder)
  glm::vec3 fillColor(0.5f, 0.5f, 0.5f);
  ImGui::Text("Fill Color:");
  ImGui::SameLine();
  ImGui::ColorEdit3("##fillColor", glm::value_ptr(fillColor),
                    ImGuiColorEditFlags_NoInputs);

  // Stroke toggle
  bool stroke = true;
  ImGui::Checkbox("Stroke", &stroke);

  // Fill toggle
  bool fill = true;
  ImGui::Checkbox("Fill", &fill);

  // Line width
  float lineWidth = 1.0f;
  ImGui::DragFloat("Line Width", &lineWidth, 0.1f, 0.1f, 10.0f, "%.1f");
}

void PropertiesPanel::renderMaterialGroup(
    std::shared_ptr<model::IFigure> figure) {
  // Material is 3D-only, show as disabled for 2D figures
  if (!ImGui::CollapsingHeader("MATERIAL")) {
    return;
  }

  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "3D Only");
  ImGui::BeginDisabled();

  glm::vec3 color(1.0f);
  float metalness = 0.0f;
  float roughness = 0.5f;

  ImGui::ColorEdit3("Color", glm::value_ptr(color));
  ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f);
  ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);

  ImGui::EndDisabled();
}

void PropertiesPanel::renderLayerGroup(std::shared_ptr<model::IFigure> figure) {
  if (!ImGui::CollapsingHeader("LAYER")) {
    return;
  }

  // Layer dropdown
  const char *layers[] = {"Default", "Layer 1", "Layer 2", "Layer 3"};
  static int currentLayer = 0;
  ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

  // Visibility
  bool visible = true;
  ImGui::Checkbox("Visible", &visible);

  // Locked
  bool locked = false;
  ImGui::Checkbox("Locked", &locked);
}

void PropertiesPanel::renderInfoGroup(std::shared_ptr<model::IFigure> figure) {
  if (!ImGui::CollapsingHeader("INFO", ImGuiTreeNodeFlags_DefaultOpen)) {
    return;
  }

  // ID (read-only)
  int id = static_cast<int>(figure->getId());
  ImGui::BeginDisabled();
  ImGui::DragInt("ID", &id);
  ImGui::EndDisabled();

  // Name (editable)
  std::string name = figure->getName();
  char nameBuffer[256];
  std::strncpy(nameBuffer, name.c_str(), sizeof(nameBuffer));
  nameBuffer[sizeof(nameBuffer) - 1] = '\0';

  if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
    fsmAdapter_.updateFigureProperty(figure->getId(), "name",
                                     std::string(nameBuffer));
  }

  // Created date (placeholder)
  ImGui::BeginDisabled();
  ImGui::Text("Created: %s", "2025-02-13");
  ImGui::EndDisabled();
}

// === Type-Specific Geometry Renderers ===

void PropertiesPanel::renderCircleGeometry(
    std::shared_ptr<model::Figure<model::Circle>> circle) {
  uint32_t figureId = circle->getId();

  // Type
  ImGui::Text("Type: Circle");

  // Center
  glm::vec3 center = pointToVec3(circle->center);
  ImGui::Text("Center:");
  ImGui::Indent();
  renderVec3Property("Center", center, "center", figureId);
  ImGui::Unindent();

  // Radius
  float radius = circle->radius;
  if (renderFloatProperty("Radius", radius, "radius", figureId)) {
    // Property updated through fsmAdapter_
  }

  // Read-only calculated properties
  if (showReadOnly_) {
    ImGui::Spacing();
    ImGui::TextDisabled("Calculated:");

    float circumference = calculateCircumference(circle->radius);
    renderReadOnlyValue("Circumference", circumference, "%.2f");

    float area = calculateCircleArea(circle->radius);
    renderReadOnlyValue("Area", area, "%.2f");
  }
}

void PropertiesPanel::renderTriangleGeometry(
    std::shared_ptr<model::Figure<model::Triangle>> triangle) {
  uint32_t figureId = triangle->getId();

  // Type
  ImGui::Text("Type: Triangle");

  // Vertices
  glm::vec3 v1 = pointToVec3(triangle->first);
  glm::vec3 v2 = pointToVec3(triangle->second);
  glm::vec3 v3 = pointToVec3(triangle->third);

  ImGui::Text("Vertex 1:");
  ImGui::Indent();
  renderVec3Property("V1", v1, "first", figureId);
  ImGui::Unindent();

  ImGui::Text("Vertex 2:");
  ImGui::Indent();
  renderVec3Property("V2", v2, "second", figureId);
  ImGui::Unindent();

  ImGui::Text("Vertex 3:");
  ImGui::Indent();
  renderVec3Property("V3", v3, "third", figureId);
  ImGui::Unindent();

  // Read-only calculated properties
  if (showReadOnly_) {
    ImGui::Spacing();
    ImGui::TextDisabled("Calculated:");

    float area = calculateTriangleArea(v1, v2, v3);
    renderReadOnlyValue("Area", area, "%.2f");
  }
}

void PropertiesPanel::renderQuadGeometry(
    std::shared_ptr<model::Figure<model::Quad>> quad) {
  uint32_t figureId = quad->getId();

  // Type
  ImGui::Text("Type: Quad");

  // Vertices
  glm::vec3 v1 = pointToVec3(quad->first);
  glm::vec3 v2 = pointToVec3(quad->second);
  glm::vec3 v3 = pointToVec3(quad->third);
  glm::vec3 v4 = pointToVec3(quad->fourth);

  ImGui::Text("Vertex 1:");
  ImGui::Indent();
  renderVec3Property("V1", v1, "first", figureId);
  ImGui::Unindent();

  ImGui::Text("Vertex 2:");
  ImGui::Indent();
  renderVec3Property("V2", v2, "second", figureId);
  ImGui::Unindent();

  ImGui::Text("Vertex 3:");
  ImGui::Indent();
  renderVec3Property("V3", v3, "third", figureId);
  ImGui::Unindent();

  ImGui::Text("Vertex 4:");
  ImGui::Indent();
  renderVec3Property("V4", v4, "fourth", figureId);
  ImGui::Unindent();

  // Read-only calculated properties
  if (showReadOnly_) {
    ImGui::Spacing();
    ImGui::TextDisabled("Calculated:");

    // Approximate area for quad (split into two triangles)
    float area =
        calculateTriangleArea(v1, v2, v3) + calculateTriangleArea(v1, v3, v4);
    renderReadOnlyValue("Area", area, "%.2f");
  }
}

void PropertiesPanel::renderNgonGeometry(
    std::shared_ptr<model::Figure<model::Ngon>> ngon) {
  uint32_t figureId = ngon->getId();

  // Type
  ImGui::Text("Type: N-gon");

  // Center
  glm::vec3 center = pointToVec3(ngon->center);
  ImGui::Text("Center:");
  ImGui::Indent();
  renderVec3Property("Center", center, "center", figureId);
  ImGui::Unindent();

  // Radius
  float radius = ngon->radius;
  renderFloatProperty("Radius", radius, "radius", figureId);

  // Number of sides
  float n = ngon->n;
  if (renderFloatProperty("Sides", n, "n", figureId)) {
    // Property updated
  }

  // Read-only calculated properties
  if (showReadOnly_) {
    ImGui::Spacing();
    ImGui::TextDisabled("Calculated:");

    float area = calculateNgonArea(ngon->radius, static_cast<int>(ngon->n));
    renderReadOnlyValue("Area", area, "%.2f");
  }
}

void PropertiesPanel::renderCurveBezier3Geometry(
    std::shared_ptr<model::Figure<model::CurveBezier3>> curve) {
  uint32_t figureId = curve->getId();

  // Type
  ImGui::Text("Type: Bezier Curve (3 control points)");

  // Start point
  glm::vec3 start = pointToVec3(curve->start);
  ImGui::Text("Start:");
  ImGui::Indent();
  renderVec3Property("Start", start, "start", figureId);
  ImGui::Unindent();

  // End point
  glm::vec3 end = pointToVec3(curve->end);
  ImGui::Text("End:");
  ImGui::Indent();
  renderVec3Property("End", end, "end", figureId);
  ImGui::Unindent();

  // Control point
  glm::vec3 cp = pointToVec3(curve->first);
  ImGui::Text("Control Point:");
  ImGui::Indent();
  renderVec3Property("CP", cp, "first", figureId);
  ImGui::Unindent();
}

void PropertiesPanel::renderCurveBezier4Geometry(
    std::shared_ptr<model::Figure<model::CurveBezier4>> curve) {
  uint32_t figureId = curve->getId();

  // Type
  ImGui::Text("Type: Bezier Curve (4 control points)");

  // Start point
  glm::vec3 start = pointToVec3(curve->start);
  ImGui::Text("Start:");
  ImGui::Indent();
  renderVec3Property("Start", start, "start", figureId);
  ImGui::Unindent();

  // End point
  glm::vec3 end = pointToVec3(curve->end);
  ImGui::Text("End:");
  ImGui::Indent();
  renderVec3Property("End", end, "end", figureId);
  ImGui::Unindent();

  // Control point 1
  glm::vec3 cp1 = pointToVec3(curve->first);
  ImGui::Text("Control Point 1:");
  ImGui::Indent();
  renderVec3Property("CP1", cp1, "first", figureId);
  ImGui::Unindent();

  // Control point 2
  glm::vec3 cp2 = pointToVec3(curve->second);
  ImGui::Text("Control Point 2:");
  ImGui::Indent();
  renderVec3Property("CP2", cp2, "second", figureId);
  ImGui::Unindent();
}

// === Property Rendering Helpers ===

bool PropertiesPanel::renderFloatProperty(const std::string &label,
                                          float &value, const std::string &path,
                                          uint32_t figureId, bool isReadOnly,
                                          float speed) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::DragFloat(("##" + label).c_str(), &value, speed);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::DragFloat(("##" + label).c_str(), &value, speed)) {
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

bool PropertiesPanel::renderVec2Property(const std::string &label,
                                         glm::vec2 &value,
                                         const std::string &path,
                                         uint32_t figureId, bool isReadOnly) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::DragFloat2(("##" + label).c_str(), glm::value_ptr(value), 0.1f);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::DragFloat2(("##" + label).c_str(), glm::value_ptr(value), 0.1f)) {
    fsmAdapter_.updateFigureProperty(figureId, path,
                                     glm::vec2(value.x, value.y));
    return true;
  }
  return false;
}

bool PropertiesPanel::renderVec3Property(const std::string &label,
                                         glm::vec3 &value,
                                         const std::string &path,
                                         uint32_t figureId, bool isReadOnly) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::DragFloat3(("##" + label).c_str(), glm::value_ptr(value), 0.1f);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::DragFloat3(("##" + label).c_str(), glm::value_ptr(value), 0.1f)) {
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

bool PropertiesPanel::renderColorProperty(const std::string &label,
                                          glm::vec3 &value,
                                          const std::string &path,
                                          uint32_t figureId, bool isReadOnly) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::ColorEdit3(("##" + label).c_str(), glm::value_ptr(value));
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::ColorEdit3(("##" + label).c_str(), glm::value_ptr(value))) {
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

bool PropertiesPanel::renderBoolProperty(const std::string &label, bool &value,
                                         const std::string &path,
                                         uint32_t figureId, bool isReadOnly) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::Checkbox(("##" + label).c_str(), &value);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::Checkbox(("##" + label).c_str(), &value)) {
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

bool PropertiesPanel::renderIntProperty(const std::string &label, int &value,
                                        const std::string &path,
                                        uint32_t figureId, bool isReadOnly) {
  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::DragInt(("##" + label).c_str(), &value);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::DragInt(("##" + label).c_str(), &value)) {
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

bool PropertiesPanel::renderStringProperty(const std::string &label,
                                           std::string &value,
                                           const std::string &path,
                                           uint32_t figureId, bool isReadOnly) {
  char buffer[256];
  std::strncpy(buffer, value.c_str(), sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0';

  if (isReadOnly) {
    ImGui::BeginDisabled();
    ImGui::InputText(("##" + label).c_str(), buffer, sizeof(buffer),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::EndDisabled();
    return false;
  }

  if (ImGui::InputText(("##" + label).c_str(), buffer, sizeof(buffer))) {
    value = buffer;
    fsmAdapter_.updateFigureProperty(figureId, path, value);
    return true;
  }
  return false;
}

void PropertiesPanel::renderReadOnlyValue(const std::string &label, float value,
                                          const char *format) {
  ImGui::BeginDisabled();
  std::string labelId = "##" + label + "_readonly";
  ImGui::Text("%s:", label.c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100);
  float v = value;
  ImGui::DragFloat(labelId.c_str(), &v, 0.0f, 0.0f, 0.0f, format);
  ImGui::EndDisabled();
}

// === Multi-Selection Support ===

void PropertiesPanel::renderMultiSelectionHeader(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  ImGui::Text("Selected Objects:");
  ImGui::Indent();

  // Show list of selected objects (limited to first 10)
  size_t count = std::min(figures.size(), static_cast<size_t>(10));
  for (size_t i = 0; i < count; ++i) {
    ImGui::BulletText("%s", figures[i]->getName().c_str());
  }

  if (figures.size() > 10) {
    ImGui::BulletText("... and %zu more", figures.size() - 10);
  }

  ImGui::Unindent();
}

void PropertiesPanel::renderMultiSelectionActions(
    const std::vector<uint32_t> &selectedIds) {
  ImGui::Separator();

  // Action buttons
  if (ImGui::Button("Group", ImVec2(80, 0))) {
    spdlog::info("Group {} objects", selectedIds.size());
  }
  ImGui::SameLine();

  if (ImGui::Button("Align", ImVec2(80, 0))) {
    spdlog::info("Align {} objects", selectedIds.size());
  }
  ImGui::SameLine();

  if (ImGui::Button("Distribute", ImVec2(80, 0))) {
    spdlog::info("Distribute {} objects", selectedIds.size());
  }

  ImGui::Spacing();

  if (ImGui::Button("Delete", ImVec2(80, 0))) {
    spdlog::info("Delete {} objects", selectedIds.size());
    // TODO: Implement delete through command manager
  }
}

std::vector<PropertiesPanel::PropertyGroup>
PropertiesPanel::buildCommonPropertyGroups(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  // TODO: Implement common property detection for multi-selection
  return {};
}

// === Utility Functions ===

std::string
PropertiesPanel::getFigureTypeName(std::shared_ptr<model::IFigure> figure) {
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

glm::vec3 PropertiesPanel::pointToVec3(const model::Point &point) {
  return glm::vec3(point.x, point.y, point.z);
}

float PropertiesPanel::calculateCircumference(float radius) {
  return 2.0f * PI * radius;
}

float PropertiesPanel::calculateCircleArea(float radius) {
  return PI * radius * radius;
}

float PropertiesPanel::calculateTriangleArea(const glm::vec3 &v1,
                                             const glm::vec3 &v2,
                                             const glm::vec3 &v3) {
  // Using cross product method: Area = 0.5 * |AB x AC|
  glm::vec3 ab = v2 - v1;
  glm::vec3 ac = v3 - v1;
  glm::vec3 cross = glm::cross(ab, ac);
  return 0.5f * glm::length(cross);
}

float PropertiesPanel::calculateLineLength(const glm::vec3 &start,
                                           const glm::vec3 &end) {
  return glm::length(end - start);
}

float PropertiesPanel::calculateLineAngle(const glm::vec3 &start,
                                          const glm::vec3 &end) {
  glm::vec3 delta = end - start;
  return glm::degrees(std::atan2(delta.y, delta.x));
}

float PropertiesPanel::calculateRectangleArea(float width, float height) {
  return width * height;
}

float PropertiesPanel::calculateRectanglePerimeter(float width, float height) {
  return 2.0f * (width + height);
}

float PropertiesPanel::calculateNgonArea(float radius, int sides) {
  // Area = (n * r^2 * sin(2π/n)) / 2
  return static_cast<float>(sides) * radius * radius *
         std::sin(2.0f * PI / static_cast<float>(sides)) / 2.0f;
}

// === Callbacks ===

void PropertiesPanel::onSelectionChanged() { propertyCache_.isValid = false; }

void PropertiesPanel::onPropertyChanged(uint32_t /*figureId*/,
                                        const std::string & /*path*/) {
  propertyCache_.isValid = false;
}

} // namespace view
