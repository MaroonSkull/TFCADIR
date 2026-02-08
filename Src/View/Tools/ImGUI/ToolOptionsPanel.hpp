#pragma once

#include <any>
#include <functional>
#include <map>
#include <string>

// Forward declaration to avoid including imgui.h in header
struct ImVec2;

namespace view {

// Forward declarations
class UIFSMAdapter;

namespace ImGUI {

/**
 * @brief ImGUI panel for tool selection and options configuration
 *
 * ToolOptionsPanel provides a stateless ImGUI interface for:
 * 1. Selecting the active drawing tool (radio buttons)
 * 2. Configuring tool-specific options (input fields)
 * 3. Visualizing current tool state
 *
 * All state is queried from the FSM via UIFSMAdapter each frame.
 * The panel does not store any tool state internally, ensuring
 * consistency with the Phase 1 architectural pattern.
 */
class ToolOptionsPanel {
public:
  /**
   * @brief Tool option value type
   */
  using OptionValue = std::any;

  /**
   * @brief Tool options map (option name -> value)
   */
  using ToolOptions = std::map<std::string, OptionValue>;

  /**
   * @brief Callback type for tool activation requests
   *
   * Called when user selects a different tool via radio button.
   * The callback should trigger the appropriate FSM event.
   */
  using ToolActivationCallback = std::function<void(const std::string &)>;

  /**
   * @brief Callback type for option value changes
   *
   * Called when user modifies a tool option value.
   * The callback should update the option in the FSM.
   */
  using OptionChangeCallback =
      std::function<void(const std::string &, const OptionValue &)>;

  /**
   * @brief Construct a ToolOptionsPanel
   * @param adapter Reference to the UIFSMAdapter for FSM queries
   */
  explicit ToolOptionsPanel(UIFSMAdapter &adapter);

  /**
   * @brief Destructor
   */
  ~ToolOptionsPanel() = default;

  // Copy prohibition
  ToolOptionsPanel(const ToolOptionsPanel &) = delete;
  ToolOptionsPanel &operator=(const ToolOptionsPanel &) = delete;

  // Move permission
  ToolOptionsPanel(ToolOptionsPanel &&) noexcept = default;
  ToolOptionsPanel &operator=(ToolOptionsPanel &&) noexcept = default;

  /**
   * @brief Render the tool options panel
   * @param panelSize The size of the panel (width, height)
   *
   * Renders the full tool options panel with:
   * 1. Tool selection section (radio buttons for all available tools)
   * 2. Options section (input fields for active tool's options)
   * 3. Status section (current tool, collected points count)
   *
   * The panel queries the FSM each frame for:
   * - Currently active tool (via getActiveTool())
   * - Active tool's options (via getToolOptions())
   * - Collected points (via getCollectedPoints())
   *
   * Example panel layout:
   * @code
   * ┌─────────────────────────────────┐
   * │ Tool Options                     │
   * ├─────────────────────────────────┤
   * │ ○ Line 3D                        │
   * │ ○ Circle 3D                      │
   * │ ● Rectangle 3D  (active)         │
   * │ ○ Polygon 3D                     │
   * ├─────────────────────────────────┤
   * │ Options:                         │
   * │ Corner: [First] [Second]         │
   * │ ├─────────────────────────────────┤
   * │ Status:                          │
   * │ Tool: Rectangle3D                │
   * │ Points: 2/4                      │
   * └─────────────────────────────────┘
   * @endcode
   */
  void render(const ImVec2 &panelSize);

  /**
   * @brief Set the callback for tool activation
   * @param callback Function to call when tool is activated
   *
   * The callback receives the tool ID to activate and should
   * trigger the appropriate FSM event (e.g., OnActivateRectangle3D).
   */
  void setToolActivationCallback(ToolActivationCallback callback) {
    onToolActivate_ = std::move(callback);
  }

  /**
   * @brief Set the callback for option changes
   * @param callback Function to call when option value changes
   *
   * The callback receives the option name and new value, and should
   * update the option in the FSM state data.
   */
  void setOptionChangeCallback(OptionChangeCallback callback) {
    onOptionChange_ = std::move(callback);
  }

  /**
   * @brief Get the panel's default size
   * @return Default panel size in pixels
   */
  static ImVec2 getDefaultSize();

private:
  /// Reference to the UIFSMAdapter for FSM queries (non-owning)
  UIFSMAdapter &adapter_;

  /// Callback for tool activation requests
  ToolActivationCallback onToolActivate_;

  /// Callback for option value changes
  OptionChangeCallback onOptionChange_;

  /**
   * @brief Available drawing tools with their display names
   */
  struct ToolInfo {
    std::string id;          ///< Tool identifier (e.g., "Line3D")
    std::string displayName; ///< Display name (e.g., "Line 3D")
    std::string event;       ///< FSM event (e.g., "OnActivateLine3D")
  };

  /// List of available drawing tools
  static const std::vector<ToolInfo> availableTools_;

  /**
   * @brief Render the tool selection section
   * @param activeTool The currently active tool ID
   *
   * Renders radio buttons for all available tools.
   * The active tool is highlighted.
   */
  void renderToolSelection(const std::string &activeTool);

  /**
   * @brief Render the tool options section
   * @param activeTool The currently active tool ID
   * @param options The current tool options
   *
   * Renders input fields for tool-specific options.
   * Different tools have different option types:
   * - Line3D: method (continuous, segment)
   * - Circle3D: radius, segments
   * - Arc3D: radius, startAngle, endAngle
   * - Rectangle3D: cornerType, fill
   * - Polygon3D: sides, radius, regular
   */
  void renderToolOptions(const std::string &activeTool,
                         const ToolOptions &options);

  /**
   * @brief Render the status section
   * @param activeTool The currently active tool ID
   * @param collectedPoints The number of collected points
   *
   * Displays current tool state and progress.
   */
  void renderStatus(const std::string &activeTool, size_t collectedPoints);

  /**
   * @brief Render a floating point option
   * @param name The option name
   * @param value The current value
   * @param minValue Minimum value constraint
   * @param maxValue Maximum value constraint
   */
  void renderFloatOption(const std::string &name, float value, float minValue,
                         float maxValue);

  /**
   * @brief Render an integer option
   * @param name The option name
   * @param value The current value
   * @param minValue Minimum value constraint
   * @param maxValue Maximum value constraint
   */
  void renderIntOption(const std::string &name, int value, int minValue,
                       int maxValue);

  /**
   * @brief Render a boolean option (checkbox)
   * @param name The option name
   * @param value The current value
   */
  void renderBoolOption(const std::string &name, bool value);

  /**
   * @brief Render an enum option (combo box)
   * @param name The option name
   * @param value The current value
   * @param options The available enum options
   */
  void renderEnumOption(const std::string &name, const std::string &value,
                        const std::vector<std::string> &options);
};

} // namespace ImGUI
} // namespace view
