#include "HelpSystem.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

namespace view {

HelpSystem::HelpSystem() = default;

HelpSystem::~HelpSystem() = default;

bool HelpSystem::initialize(const std::string &configPath) {
  if (initialized_) {
    spdlog::warn("HelpSystem already initialized");
    return true;
  }

  // Note: YAML loading not implemented - using hardcoded defaults
  // To enable customization, integrate yaml-cpp and implement YAML loading
  spdlog::info("Loading default help topics (YAML loading not implemented)");
  loadDefaults();

  buildContextMap();
  initialized_ = true;
  return true;
}

void HelpSystem::loadDefaults() {
  loadDefaultShortcuts();
  loadDefaultTutorials();
  loadDefaultTooltips();

  // Add default help topics
  topics_ = {{"getting_started",
              "Getting Started",
              "Getting Started",
              "Welcome to TFCADIR CAD application!\n\n"
              "Here are some resources to help you get started:\n"
              "- Press F1 to open context-sensitive help\n"
              "- Browse documentation from Help menu\n"
              "- Complete interactive tutorials\n\n"
              "Basic Navigation:\n"
              "- Use mouse wheel to zoom in/out\n"
              "- Right-click and drag to pan\n"
              "- Middle mouse button to orbit (3D mode)\n\n"
              "Drawing:\n"
              "- Select tools from the Tools panel\n"
              "- Click on canvas to place points\n"
              "- Use coordinate input for precise placement",
              {"start", "begin", "introduction", "welcome", "new"},
              {}},

             {"keyboard_shortcuts",
              "Keyboard Shortcuts",
              "Reference",
              "Keyboard Shortcuts Reference\n\n"
              "File Operations:\n"
              "  Ctrl+N - New Project\n"
              "  Ctrl+O - Open Project\n"
              "  Ctrl+S - Save Project\n\n"
              "Edit Operations:\n"
              "  Ctrl+Z - Undo\n"
              "  Ctrl+Y - Redo\n"
              "  Ctrl+X - Cut\n"
              "  Ctrl+C - Copy\n"
              "  Ctrl+V - Paste\n"
              "  Delete - Delete Selected\n\n"
              "View Controls:\n"
              "  F2 - 2D Mode\n"
              "  F3 - 3D Mode\n"
              "  F9 - Keyboard Shortcuts Dialog\n"
              "  F1 - Context Help\n\n"
              "Drawing Tools:\n"
              "  L - Line Tool\n"
              "  C - Circle Tool\n"
              "  R - Rectangle Tool\n\n"
              "Selection:\n"
              "  Ctrl+A - Select All\n"
              "  Ctrl+I - Invert Selection\n"
              "  Escape - Clear Selection",
              {"keys", "hotkeys", "shortcuts", "keyboard"},
              {}},

             {"mouse_navigation",
              "Mouse & Navigation",
              "Navigation",
              "Mouse Controls\n\n"
              "View Navigation:\n"
              "- Mouse Wheel: Zoom in/out\n"
              "- Right-click + Drag: Pan the view\n"
              "- Middle Mouse + Drag: Orbit (3D mode)\n\n"
              "Selection:\n"
              "- Left-click: Select object\n"
              "- Ctrl + Left-click: Add to selection\n"
              "- Shift + Left-click: Range selection\n"
              "- Left-click + Drag: Box selection\n\n"
              "Context Menus:\n"
              "- Right-click on canvas: Canvas context menu\n"
              "- Right-click on object: Object context menu\n\n"
              "Measurement:\n"
              "- Hover over objects to see dimensions\n"
              "- Use measurement tools for precise values",
              {"mouse", "navigation", "pan", "zoom", "orbit", "view"},
              {}},

             {"tools_reference",
              "Tools Reference",
              "Reference",
              "Drawing Tools\n\n"
              "Line Tool (L):\n"
              "Draw straight line segments between two points.\n"
              "Click to set start point, click again to set end point.\n\n"
              "Circle Tool (C):\n"
              "Draw circles by center and radius.\n"
              "Click to set center, drag to set radius.\n\n"
              "Rectangle Tool (R):\n"
              "Draw rectangles by two corner points.\n"
              "Click to set first corner, drag to set opposite corner.\n\n"
              "Annotation Tools:\n"
              "- Linear Dimension: Measure distance between two points\n"
              "- Angular Dimension: Measure angle between two lines\n"
              "- Radial Dimension: Show radius of circles/arcs\n"
              "- Diameter Dimension: Show diameter of circles/arcs\n"
              "- Leader Line: Add leader with text annotation\n"
              "- Text Annotation: Add text labels\n\n"
              "Measurement Tools:\n"
              "- Distance: Measure distance between points\n"
              "- Area: Calculate area of polygon\n"
              "- Angle: Measure angle between three points\n"
              "- Coordinate: Display point coordinates\n"
              "- Length: Measure length along path",
              {"tools", "drawing", "line", "circle", "rectangle", "annotation",
               "measurement"},
              {}},

             {"view_controls",
              "View Controls",
              "Navigation",
              "View Presets\n\n"
              "Quick view presets are available via NumPad keys:\n"
              "- NumPad 1: Front view (Ctrl+NumPad 1: Back)\n"
              "- NumPad 3: Right view (Ctrl+NumPad 3: Left)\n"
              "- NumPad 7: Top view (Ctrl+NumPad 7: Bottom)\n"
              "- NumPad 5: Isometric view\n\n"
              "View Modes:\n"
              "- F2: Switch to 2D drafting mode\n"
              "- F3: Switch to 3D modeling mode\n\n"
              "Grid & Snap:\n"
              "- Toggle grid visibility from View menu\n"
              "- Configure snap settings in Snap Settings panel\n"
              "- Use coordinate input for precise placement\n\n"
              "Display Settings:\n"
              "- Configure theme from View > Display Settings\n"
              "- Adjust grid spacing and style\n"
              "- Set measurement units",
              {"view", "presets", "camera", "grid", "snap", "display"},
              {}},

             {"sketch_mode",
              "Sketch Mode",
              "Modes",
              "Sketch Mode\n\n"
              "Sketch mode provides a 2D drawing environment for creating\n"
              "precise technical drawings.\n\n"
              "Entering Sketch Mode:\n"
              "1. Select Sketch > Enter Sketch Mode from menu\n"
              "2. Select a sketch plane (XY, XZ, or YZ)\n"
              "3. Begin drawing on the selected plane\n\n"
              "Available in Sketch Mode:\n"
              "- All 2D drawing tools\n"
              "- Dimension and annotation tools\n"
              "- Grid and snap assistance\n"
              "- Coordinate input widget\n\n"
              "Exiting Sketch Mode:\n"
              "Select Sketch > Exit Sketch Mode from menu\n\n"
              "Note: Sketch mode is ideal for creating 2D profiles\n"
              "that can later be extruded into 3D objects.",
              {"sketch", "2d", "drawing", "plane"},
              {}},

             {"selection_tools",
              "Selection Tools",
              "Tools",
              "Selection Modes\n\n"
              "Point Selection (1):\n"
              "Click on individual objects to select them.\n"
              "Hold Ctrl to add to selection.\n\n"
              "Box Selection (2):\n"
              "Click and drag to create a selection box.\n"
              "All objects inside the box are selected.\n\n"
              "Lasso Selection (3):\n"
              "Draw a freeform lasso around objects.\n"
              "Release to select enclosed objects.\n\n"
              "Polygon Selection (4):\n"
              "Click to place polygon vertices.\n"
              "Double-click or press Enter to complete.\n\n"
              "Selection Filters:\n"
              "Use Selection Settings panel to filter by:\n"
              "- Object type (triangles, circles, etc.)\n"
              "- Visibility status\n"
              "- Lock status\n\n"
              "Selection Memory:\n"
              "Save and restore selection sets for quick access.",
              {"selection", "select", "box", "lasso", "polygon", "filter"},
              {}},

             {"layer_management",
              "Layer Management",
              "Organization",
              "Layer Manager\n\n"
              "Layers help organize your drawing by grouping\n"
              "related objects together.\n\n"
              "Layer Properties:\n"
              "- Name: Descriptive layer name\n"
              "- Visibility: Show/hide layer contents\n"
              "- Lock: Prevent editing of layer objects\n"
              "- Color: Default color for new objects\n\n"
              "Layer Operations:\n"
              "- Create new layer: Click '+' button\n"
              "- Delete layer: Select and click '-' button\n"
              "- Rename: Double-click layer name\n"
              "- Set active: Single-click layer\n\n"
              "Object Assignment:\n"
              "Select objects and use context menu to assign\n"
              "them to a different layer.\n\n"
              "Tip: Use layers to separate different types of\n"
              "geometry (construction, dimensions, etc.)",
              {"layer", "layers", "organize", "visibility", "lock"},
              {}},

             {"import_export",
              "Import/Export",
              "File Operations",
              "Import Formats\n\n"
              "DXF Import:\n"
              "1. File > Import...\n"
              "2. Select DXF file\n"
              "3. Configure import options\n"
              "4. Click Import\n\n"
              "SVG Import:\n"
              "1. File > Import...\n"
              "2. Select SVG file\n"
              "3. Configure scale and units\n"
              "4. Click Import\n\n"
              "Export Formats\n\n"
              "DXF Export:\n"
              "1. Select objects (or export all)\n"
              "2. File > Export...\n"
              "3. Choose DXF format\n"
              "4. Configure export options\n"
              "5. Click Export\n\n"
              "SVG Export:\n"
              "1. Select objects (or export all)\n"
              "2. File > Export...\n"
              "3. Choose SVG format\n"
              "4. Configure scale and style\n"
              "5. Click Export",
              {"import", "export", "dxf", "svg", "file"},
              {}},

             {"context_menu",
              "Context Menus",
              "Interface",
              "Canvas Context Menu\n\n"
              "Right-click on empty canvas space to access:\n"
              "- Pan: Start pan operation\n"
              "- Zoom to Fit: Fit all objects in view\n"
              "- Zoom to Selection: Fit selected objects\n"
              "- Snap Settings: Open snap configuration\n"
              "- Grid Settings: Open grid configuration\n\n"
              "Object Context Menu\n\n"
              "Right-click on selected object(s) to access:\n"
              "- Duplicate: Create copy of selection\n"
              "- Delete: Remove selected objects\n"
              "- Move: Start move operation\n"
              "- Rotate: Start rotate operation\n"
              "- Scale: Start scale operation\n"
              "- Properties: Open properties panel\n"
              "- Layers: Assign to layer\n"
              "- Bring to Front: Move to top of z-order\n"
              "- Send to Back: Move to bottom of z-order\n"
              "- Create Copy: Copy with base point\n"
              "- Mirror: Mirror across axis",
              {"context", "menu", "right-click", "popup"},
              {}}};
}

void HelpSystem::loadDefaultShortcuts() {
  shortcuts_ = {
      // File Operations
      {"Ctrl+N", "New Project", "Create a new project file", "File Operations"},
      {"Ctrl+O", "Open Project", "Open an existing project", "File Operations"},
      {"Ctrl+S", "Save Project", "Save current project", "File Operations"},
      {"Ctrl+Shift+S", "Save As...", "Save project with new name",
       "File Operations"},

      // Edit Operations
      {"Ctrl+Z", "Undo", "Undo last action", "Edit Operations"},
      {"Ctrl+Y", "Redo", "Redo last undone action", "Edit Operations"},
      {"Ctrl+X", "Cut", "Cut selection to clipboard", "Edit Operations"},
      {"Ctrl+C", "Copy", "Copy selection to clipboard", "Edit Operations"},
      {"Ctrl+V", "Paste", "Paste from clipboard", "Edit Operations"},
      {"Delete", "Delete", "Delete selected objects", "Edit Operations"},
      {"Ctrl+A", "Select All", "Select all objects", "Edit Operations"},
      {"Ctrl+I", "Invert Selection", "Invert current selection",
       "Edit Operations"},

      // View Controls
      {"F1", "Help", "Open context-sensitive help", "View Controls"},
      {"F2", "2D Mode", "Switch to 2D drafting mode", "View Controls"},
      {"F3", "3D Mode", "Switch to 3D modeling mode", "View Controls"},
      {"F9", "Shortcuts", "Open keyboard shortcuts dialog", "View Controls"},
      {"NumPad 1", "Front View", "Set front view (Ctrl for back)",
       "View Controls"},
      {"NumPad 3", "Right View", "Set right view (Ctrl for left)",
       "View Controls"},
      {"NumPad 5", "Isometric View", "Set isometric view", "View Controls"},
      {"NumPad 7", "Top View", "Set top view (Ctrl for bottom)",
       "View Controls"},

      // Drawing Tools
      {"L", "Line Tool", "Draw straight lines", "Drawing Tools"},
      {"C", "Circle Tool", "Draw circles", "Drawing Tools"},
      {"R", "Rectangle Tool", "Draw rectangles", "Drawing Tools"},
      {"A", "Arc Tool", "Draw arcs", "Drawing Tools"},
      {"P", "Polygon Tool", "Draw polygons", "Drawing Tools"},

      // Measurement Tools
      {"M", "Distance Measure", "Measure distance between points",
       "Measurement"},
      {"Shift+A", "Area Measure", "Calculate area of polygon", "Measurement"},

      // Selection
      {"1", "Point Selection", "Point selection mode", "Selection"},
      {"2", "Box Selection", "Box selection mode", "Selection"},
      {"3", "Lasso Selection", "Lasso selection mode", "Selection"},
      {"4", "Polygon Selection", "Polygon selection mode", "Selection"},
      {"Escape", "Cancel/Deselect", "Cancel operation or clear selection",
       "Selection"},

      // Sketch Mode
      {"S", "Enter Sketch", "Enter sketch mode", "Sketch Mode"},
      {"Escape", "Exit Sketch", "Exit current mode", "Sketch Mode"}};
}

void HelpSystem::loadDefaultTutorials() {
  tutorials_ = {
      {"getting_started",
       "Getting Started with TFCADIR",
       "Learn the basics of TFCADIR CAD application",
       {{"Welcome",
         "Welcome to TFCADIR! This tutorial will guide you through the basic "
         "features.\n\nClick 'Next' to continue.",
         "Press F1 anytime for context-sensitive help"},
        {"Navigation",
         "Use the mouse to navigate the view:\n\n"
         "- Mouse wheel: Zoom in/out\n"
         "- Right-click + drag: Pan\n"
         "- Middle mouse + drag: Orbit (3D mode)\n\n"
         "Try zooming and panning now.",
         "NumPad keys provide quick view presets"},
        {"Tools Panel",
         "The Tools panel on the right contains drawing tools.\n\n"
         "Select a tool by clicking on it, then click on the canvas to draw.",
         "Press L for Line, C for Circle, R for Rectangle"},
        {"Selection",
         "Click on objects to select them.\n\n"
         "- Hold Ctrl to add to selection\n"
         "- Press Delete to remove selected objects\n"
         "- Press Escape to clear selection",
         "Use box selection by clicking and dragging on empty space"},
        {"Complete",
         "Congratulations! You've completed the Getting Started tutorial.\n\n"
         "Explore other tutorials to learn more features.",
         "Press F1 for help anytime"}},
       0,
       false},

      {"drawing_basics",
       "Basic Drawing",
       "Learn how to create basic shapes",
       {{"Line Tool",
         "The Line tool (L) draws straight line segments.\n\n"
         "1. Press L or click Line button\n"
         "2. Click to set start point\n"
         "3. Click to set end point",
         "Use coordinate input for precise placement"},
        {"Circle Tool",
         "The Circle tool (C) draws circles.\n\n"
         "1. Press C or click Circle button\n"
         "2. Click to set center\n"
         "3. Drag to set radius",
         "Hold Shift for perfect circles"},
        {"Rectangle Tool",
         "The Rectangle tool (R) draws rectangles.\n\n"
         "1. Press R or click Rectangle button\n"
         "2. Click to set first corner\n"
         "3. Drag to set opposite corner",
         "Hold Shift for squares"},
        {"Complete",
         "You've learned the basic drawing tools!\n\n"
         "Practice drawing different shapes to become familiar with the tools.",
         "Try combining shapes to create complex drawings"}},
       0,
       false},

      {"precision_drawing",
       "Precision Drawing",
       "Learn to use grid, snap, and coordinate input",
       {{"Grid Settings",
         "The grid helps you align objects precisely.\n\n"
         "Open Grid Settings panel to configure:\n"
         "- Grid spacing\n"
         "- Grid style (lines or dots)\n"
         "- Major grid lines",
         "Toggle grid visibility from View menu"},
        {"Snap Settings",
         "Snap helps you place points accurately.\n\n"
         "Available snap modes:\n"
         "- Grid snap\n"
         "- Endpoint snap\n"
         "- Midpoint snap\n"
         "- Center snap\n"
         "- Intersection snap",
         "Toggle snap modes in Snap Settings panel"},
        {"Coordinate Input",
         "For precise placement, use the Coordinate Input widget.\n\n"
         "Enter exact X, Y, Z coordinates for the next point.\n"
         "Press Enter to confirm.",
         "Use relative coordinates with @ prefix (e.g., @10,5)"},
        {"Complete",
         "You've learned precision drawing techniques!\n\n"
         "Use these tools together for accurate technical drawings.",
         "Combine grid, snap, and coordinate input for best results"}},
       0,
       false}};
}

void HelpSystem::loadDefaultTooltips() {
  tooltips_ = {
      // Tool buttons
      {"tool_line", "Line Tool (L) - Draw straight line segments"},
      {"tool_circle", "Circle Tool (C) - Draw circles by center and radius"},
      {"tool_rectangle", "Rectangle Tool (R) - Draw rectangles"},
      {"tool_arc", "Arc Tool (A) - Draw arcs"},
      {"tool_polygon", "Polygon Tool (P) - Draw polygons"},

      // View controls
      {"view_zoom_in", "Zoom In - Increase magnification"},
      {"view_zoom_out", "Zoom Out - Decrease magnification"},
      {"view_zoom_fit", "Zoom to Fit - Fit all objects in view"},
      {"view_pan", "Pan - Move the view"},
      {"view_orbit", "Orbit - Rotate the 3D view"},

      // Selection
      {"selection_point",
       "Point Selection - Click to select individual objects"},
      {"selection_box", "Box Selection - Drag to select multiple objects"},
      {"selection_lasso", "Lasso Selection - Draw freeform selection area"},
      {"selection_polygon", "Polygon Selection - Draw polygon selection area"},

      // Layer controls
      {"layer_add", "Add Layer - Create a new layer"},
      {"layer_delete", "Delete Layer - Remove selected layer"},
      {"layer_visible", "Toggle Visibility - Show/hide layer contents"},
      {"layer_lock", "Toggle Lock - Lock/unlock layer for editing"},

      // Measurement
      {"measure_distance", "Distance - Measure distance between two points"},
      {"measure_area", "Area - Calculate area of polygon"},
      {"measure_angle", "Angle - Measure angle between three points"},

      // Annotation
      {"annotation_linear", "Linear Dimension - Add linear dimension"},
      {"annotation_angular", "Angular Dimension - Add angular dimension"},
      {"annotation_radial", "Radial Dimension - Add radial dimension"},
      {"annotation_diameter", "Diameter Dimension - Add diameter dimension"},
      {"annotation_leader", "Leader Line - Add leader with text"},
      {"annotation_text", "Text Annotation - Add text label"},

      // File operations
      {"file_new", "New Project (Ctrl+N) - Create new project"},
      {"file_open", "Open Project (Ctrl+O) - Open existing project"},
      {"file_save", "Save Project (Ctrl+S) - Save current project"},
      {"file_import", "Import - Import DXF or SVG file"},
      {"file_export", "Export - Export to DXF or SVG"},

      // Edit operations
      {"edit_undo", "Undo (Ctrl+Z) - Undo last action"},
      {"edit_redo", "Redo (Ctrl+Y) - Redo last undone action"},
      {"edit_cut", "Cut (Ctrl+X) - Cut selection to clipboard"},
      {"edit_copy", "Copy (Ctrl+C) - Copy selection to clipboard"},
      {"edit_paste", "Paste (Ctrl+V) - Paste from clipboard"},
      {"edit_delete", "Delete - Delete selected objects"},

      // Mode switches
      {"mode_2d", "2D Mode (F2) - Switch to 2D drafting mode"},
      {"mode_3d", "3D Mode (F3) - Switch to 3D modeling mode"},
      {"mode_sketch", "Sketch Mode - Enter sketch mode for 2D drawing"},

      // Help
      {"help_button", "Help (F1) - Open context-sensitive help"},
      {"shortcuts_button", "Shortcuts (F9) - Open keyboard shortcuts dialog"}};
}

void HelpSystem::buildContextMap() {
  // Map context identifiers to topic IDs
  contextToTopic_ = {// Tools
                     {"tool.line", "tools_reference"},
                     {"tool.circle", "tools_reference"},
                     {"tool.rectangle", "tools_reference"},
                     {"tool.arc", "tools_reference"},
                     {"tool.polygon", "tools_reference"},

                     // Modes
                     {"mode.sketch", "sketch_mode"},
                     {"mode.2d", "view_controls"},
                     {"mode.3d", "view_controls"},
                     {"plane_selection", "tools_reference"},

                     // Operations
                     {"selection", "selection_tools"},
                     {"navigation", "mouse_navigation"},
                     {"import_export", "import_export"},

                     // General
                     {"default", "getting_started"},
                     {"shortcuts", "keyboard_shortcuts"},
                     {"help", "getting_started"}};
}

void HelpSystem::showHelp(const std::string &context) {
  spdlog::info("Showing help for context: {}", context);

  if (helpDialogCallback_) {
    helpDialogCallback_(true);
  }
}

std::string HelpSystem::getHelpText(const std::string &context) const {
  // Look up topic for context
  auto it = contextToTopic_.find(context);
  std::string topicId =
      (it != contextToTopic_.end()) ? it->second : "getting_started";

  // Find the topic
  for (const auto &topic : topics_) {
    if (topic.id == topicId) {
      return topic.content;
    }
  }

  return "Help content not found for context: " + context;
}

bool HelpSystem::hasHelp(const std::string &context) const {
  return contextToTopic_.find(context) != contextToTopic_.end();
}

const std::vector<HelpContentTopic> &HelpSystem::getAllTopics() const {
  return topics_;
}

std::vector<HelpContentTopic>
HelpSystem::getTopicsByCategory(const std::string &category) const {
  std::vector<HelpContentTopic> result;
  for (const auto &topic : topics_) {
    if (topic.category == category) {
      result.push_back(topic);
    }
  }
  return result;
}

std::vector<HelpContentTopic>
HelpSystem::searchTopics(const std::string &query) const {
  std::vector<HelpContentTopic> result;
  std::string lowerQuery = toLower(query);

  for (const auto &topic : topics_) {
    // Search in title
    std::string lowerTitle = toLower(topic.title);
    if (lowerTitle.find(lowerQuery) != std::string::npos) {
      result.push_back(topic);
      continue;
    }

    // Search in content
    std::string lowerContent = toLower(topic.content);
    if (lowerContent.find(lowerQuery) != std::string::npos) {
      result.push_back(topic);
      continue;
    }

    // Search in keywords
    for (const auto &keyword : topic.keywords) {
      std::string lowerKeyword = toLower(keyword);
      if (lowerKeyword.find(lowerQuery) != std::string::npos) {
        result.push_back(topic);
        break;
      }
    }
  }

  return result;
}

const std::vector<HelpShortcutEntry> &HelpSystem::getAllShortcuts() const {
  return shortcuts_;
}

std::vector<HelpShortcutEntry>
HelpSystem::getShortcutsByCategory(const std::string &category) const {
  std::vector<HelpShortcutEntry> result;
  for (const auto &shortcut : shortcuts_) {
    if (shortcut.category == category) {
      result.push_back(shortcut);
    }
  }
  return result;
}

std::vector<std::string> HelpSystem::getTutorialIds() const {
  std::vector<std::string> ids;
  for (const auto &tutorial : tutorials_) {
    ids.push_back(tutorial.id);
  }
  return ids;
}

const HelpTutorial *HelpSystem::getTutorial(const std::string &id) const {
  for (const auto &tutorial : tutorials_) {
    if (tutorial.id == id) {
      return &tutorial;
    }
  }
  return nullptr;
}

bool HelpSystem::startTutorial(const std::string &id) {
  for (size_t i = 0; i < tutorials_.size(); ++i) {
    if (tutorials_[i].id == id) {
      currentTutorialIndex_ = static_cast<int>(i);
      tutorials_[i].currentStep = 0;
      tutorials_[i].completed = false;
      for (auto &step : tutorials_[i].steps) {
        step.completed = false;
      }
      spdlog::info("Started tutorial: {}", id);
      return true;
    }
  }
  spdlog::warn("Tutorial not found: {}", id);
  return false;
}

bool HelpSystem::advanceTutorial() {
  if (currentTutorialIndex_ < 0 ||
      currentTutorialIndex_ >= static_cast<int>(tutorials_.size())) {
    return false;
  }

  auto &tutorial = tutorials_[currentTutorialIndex_];

  // Mark current step as complete
  if (tutorial.currentStep < tutorial.steps.size()) {
    tutorial.steps[tutorial.currentStep].completed = true;
  }

  // Advance to next step
  tutorial.currentStep++;

  // Check if tutorial is complete
  if (tutorial.currentStep >= tutorial.steps.size()) {
    tutorial.completed = true;
    spdlog::info("Tutorial completed: {}", tutorial.id);
    return false;
  }

  return true;
}

const HelpTutorial *HelpSystem::getCurrentTutorial() const {
  if (currentTutorialIndex_ < 0 ||
      currentTutorialIndex_ >= static_cast<int>(tutorials_.size())) {
    return nullptr;
  }
  return &tutorials_[currentTutorialIndex_];
}

bool HelpSystem::isTutorialActive() const { return currentTutorialIndex_ >= 0; }

void HelpSystem::cancelTutorial() {
  if (currentTutorialIndex_ >= 0 &&
      currentTutorialIndex_ < static_cast<int>(tutorials_.size())) {
    spdlog::info("Cancelled tutorial: {}",
                 tutorials_[currentTutorialIndex_].id);
  }
  currentTutorialIndex_ = -1;
}

std::string HelpSystem::getTooltip(const std::string &elementId) const {
  auto it = tooltips_.find(elementId);
  if (it != tooltips_.end()) {
    return it->second;
  }
  return "";
}

void HelpSystem::registerTooltip(const std::string &elementId,
                                 const std::string &text) {
  tooltips_[elementId] = text;
}

std::vector<std::string> HelpSystem::getCategories() const {
  std::vector<std::string> categories;
  for (const auto &topic : topics_) {
    if (std::find(categories.begin(), categories.end(), topic.category) ==
        categories.end()) {
      categories.push_back(topic.category);
    }
  }
  return categories;
}

void HelpSystem::setHelpDialogCallback(std::function<void(bool)> callback) {
  helpDialogCallback_ = std::move(callback);
}

std::string HelpSystem::toLower(const std::string &str) {
  std::string result;
  result.reserve(str.size());
  std::transform(str.begin(), str.end(), std::back_inserter(result),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

} // namespace view
