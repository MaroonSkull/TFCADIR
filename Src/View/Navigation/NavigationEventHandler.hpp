#pragma once

#include <View/Navigation/NavigationTypes.hpp>
#include <glm/glm.hpp>

namespace view {

// Forward declarations
class NavigationManager;
class ToolManager;

/**
 * @brief Handles mouse and keyboard events for navigation
 * @details Processes input events and delegates to NavigationManager for
 *          navigation operations. Supports mouse wheel zoom, middle mouse
 *          pan/orbit, and tool-specific navigation handling.
 */
class NavigationEventHandler {
public:
  /**
   * @brief Constructor for NavigationEventHandler
   * @param navigationManager Reference to NavigationManager
   * @param toolManager Pointer to ToolManager (optional, for tool-specific
   * handling)
   */
  NavigationEventHandler(NavigationManager &navigationManager,
                         ToolManager *toolManager = nullptr);

  /**
   * @brief Destructor
   */
  ~NavigationEventHandler() = default;

  /**
   * @brief Handles mouse wheel scroll event for zooming
   * @param delta Scroll delta (positive = scroll up/zoom in, negative = scroll
   * down/zoom out)
   * @param cursorPos Current cursor position in screen coordinates
   * @param viewportSize Viewport size in pixels
   * @return true if the event was handled
   */
  bool handleMouseWheel(float delta, const glm::ivec2 &cursorPos,
                        const glm::ivec2 &viewportSize);

  /**
   * @brief Handles mouse button press event
   * @param button Mouse button (0=left, 1=right, 2=middle)
   * @param mods Modifier keys (shift, ctrl, alt)
   * @param cursorPos Current cursor position in screen coordinates
   * @return true if the event was handled
   */
  bool handleMouseButtonPress(int button, int mods,
                              const glm::ivec2 &cursorPos);

  /**
   * @brief Handles mouse button release event
   * @param button Mouse button (0=left, 1=right, 2=middle)
   * @param cursorPos Current cursor position in screen coordinates
   * @return true if the event was handled
   */
  bool handleMouseButtonRelease(int button, const glm::ivec2 &cursorPos);

  /**
   * @brief Handles mouse move event
   * @param cursorPos Current cursor position in screen coordinates
   * @return true if the event was handled
   */
  bool handleMouseMove(const glm::ivec2 &cursorPos);

  /**
   * @brief Handles keyboard key press event
   * @param key Keyboard key code
   * @param mods Modifier keys (shift, ctrl, alt)
   * @return true if the event was handled
   */
  bool handleKeyPress(int key, int mods);

  /**
   * @brief Checks if navigation is currently active
   * @return true if currently navigating (panning, orbiting, or zooming)
   */
  [[nodiscard]] bool isNavigating() const;

  /**
   * @brief Gets the current navigation mode
   * @return Current navigation mode
   */
  [[nodiscard]] NavigationMode getNavigationMode() const;

private:
  /// Reference to NavigationManager
  NavigationManager &navigationManager_;

  /// Pointer to ToolManager (optional, for tool-specific handling)
  ToolManager *toolManager_;

  /// Last cursor position for delta calculations
  glm::ivec2 lastCursorPos_;

  /// Start cursor position for navigation operations
  glm::ivec2 startCursorPos_;

  /// Flag to track if middle mouse button is pressed
  bool isMiddleMousePressed_;

  /// Flag to track if left mouse button is pressed (for zoom box)
  bool isLeftMousePressed_;

  /// Flag to track if currently navigating
  bool isNavigating_;

  /**
   * @brief Checks if navigation should be allowed based on current tool
   * @return true if navigation is allowed
   */
  [[nodiscard]] bool isNavigationAllowed() const;

  /**
   * @brief Determines navigation mode based on current state
   * @return The appropriate navigation mode
   */
  [[nodiscard]] NavigationMode determineNavigationMode() const;
};

} // namespace view
