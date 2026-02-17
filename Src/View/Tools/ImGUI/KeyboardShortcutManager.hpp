#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

// Forward declaration to avoid including imgui.h in header
struct ImGuiIO;

namespace view {
namespace ImGUI {

/**
 * @brief Keyboard shortcut manager for ImGUI interface
 *
 * KeyboardShortcutManager handles keyboard shortcuts for tool activation
 * and common operations. All shortcuts use Ctrl+ modifier to prevent
 * conflicts with single-letter text input in the application.
 *
 * The manager checks ImGuiIO each frame for registered key combinations
 * and invokes the appropriate callbacks. This is a stateless design -
 * no shortcut state is stored internally between frames.
 */
class KeyboardShortcutManager {
public:
  /**
   * @brief Keyboard key identifier
   */
  enum class Key : int {
    None = 0,
    // Letters A-Z
    A = 0x41,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    // Numbers 0-9
    Num0 = 0x30,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    // Function keys
    F1 = 0x70,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    // Special keys
    Space = 0x20,
    Enter = 0x0D,
    Escape = 0x1B,
    Backspace = 0x08,
    Tab = 0x09,
    Insert = 0x2D,
    Delete = 0x2E,
    Home = 0x24,
    End = 0x23,
    PageUp = 0x21,
    PageDown = 0x22,
    ArrowUp = 0x26,
    ArrowDown = 0x28,
    ArrowLeft = 0x25,
    ArrowRight = 0x27
  };

  /**
   * @brief Modifier key flags
   */
  enum class Modifier : int {
    None = 0,
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2
  };

  /**
   * @brief Callback type for shortcut activation
   */
  using ShortcutCallback = std::function<void()>;

  /**
   * @brief Construct a KeyboardShortcutManager
   */
  KeyboardShortcutManager() = default;

  /**
   * @brief Destructor
   */
  ~KeyboardShortcutManager() = default;

  // Copy prohibition
  KeyboardShortcutManager(const KeyboardShortcutManager &) = delete;
  KeyboardShortcutManager &operator=(const KeyboardShortcutManager &) = delete;

  // Move permission
  KeyboardShortcutManager(KeyboardShortcutManager &&) noexcept = default;
  KeyboardShortcutManager &
  operator=(KeyboardShortcutManager &&) noexcept = default;

  /**
   * @brief Register a keyboard shortcut
   * @param key The primary key (e.g., Key::L for Ctrl+L)
   * @param modifiers Modifier key flags (e.g., Modifier::Ctrl)
   * @param callback Function to call when shortcut is triggered
   * @param description Human-readable description for help display
   *
   * Registers a new keyboard shortcut. The shortcut will be checked each
   * frame when handleShortcut() is called.
   *
   * IMPORTANT: All shortcuts MUST use at least the Ctrl+ modifier.
   * Single-letter shortcuts (e.g., just 'L') are PROHIBITED to prevent
   * conflicts with text input throughout the application.
   *
   * Example:
   * @code
   * manager.registerShortcut(Key::L, Modifier::Ctrl, []() {
   *     toolManager.activateTool("Line3D");
   * }, "Activate Line 3D tool");
   * @endcode
   *
   * @throws std::invalid_argument if modifiers is Modifier::None
   */
  void registerShortcut(Key key, Modifier modifiers, ShortcutCallback callback,
                        const std::string &description);

  /**
   * @brief Unregister a keyboard shortcut
   * @param key The primary key of the shortcut to unregister
   * @param modifiers The modifier flags of the shortcut to unregister
   */
  void unregisterShortcut(Key key, Modifier modifiers);

  /**
   * @brief Handle keyboard input for registered shortcuts
   * @param io Reference to ImGuiIO for checking key states
   *
   * Checks all registered shortcuts against the current ImGuiIO state.
   * If a matching shortcut is found, its callback is invoked.
   *
   * This method queries ImGuiIO each frame and stores no internal state,
   * ensuring the stateless design pattern required by Phase 2 architecture.
   *
   * Should be called once per frame in the main render loop:
   * @code
   * void renderFrame() {
   *     ImGuiIO& io = ImGui::GetIO();
   *     keyboardShortcutManager.handleShortcut(io);
   *     // ... rest of rendering
   * }
   * @endcode
   */
  void handleShortcut(const ImGuiIO &io);

  /**
   * @brief Clear all registered shortcuts
   */
  void clear();

  /**
   * @brief Get a list of all registered shortcuts with descriptions
   * @return Vector of strings describing each shortcut
   *
   * Returns human-readable descriptions of all registered shortcuts,
   * useful for displaying in a help panel or keyboard shortcuts dialog.
   *
   * Example output:
   * @code
   * {
   *     "Ctrl+L: Activate Line 3D tool",
   *     "Ctrl+C: Activate Circle 3D tool",
   *     "Ctrl+R: Activate Rectangle 3D tool",
   *     "Ctrl+Z: Undo",
   *     "Ctrl+Shift+Z: Redo"
   * }
   * @endcode
   */
  std::vector<std::string> getShortcutDescriptions() const;

private:
  /**
   * @brief Shortcut registration information
   */
  struct Shortcut {
    Key key;                   ///< Primary key
    Modifier modifiers;        ///< Modifier flags
    ShortcutCallback callback; ///< Function to invoke
    std::string description;   ///< Human-readable description
  };

  /// Registered shortcuts (key + modifiers -> shortcut info)
  std::map<std::pair<Key, Modifier>, Shortcut> shortcuts_;

  /**
   * @brief Check if a key combination matches a shortcut
   * @param io Reference to ImGuiIO for checking key states
   * @param shortcut The shortcut to check
   * @return true if the shortcut is currently pressed
   *
   * Checks if:
   * 1. The primary key is pressed (io.KeysDown[key])
   * 2. All required modifiers are pressed
   * 3. No other modifiers are pressed (unless specified)
   */
  bool isShortcutPressed(const ImGuiIO &io, const Shortcut &shortcut) const;

  /**
   * @brief Convert a key enum to string representation
   * @param key The key to convert
   * @return String representation of the key
   */
  static std::string keyToString(Key key);

  /**
   * @brief Convert modifier flags to string representation
   * @param modifiers The modifier flags to convert
   * @return String representation of the modifiers (e.g., "Ctrl+Shift+")
   */
  static std::string modifiersToString(Modifier modifiers);

  /**
   * @brief Combine key and modifiers into a string description
   * @param key The primary key
   * @param modifiers The modifier flags
   * @param description The action description
   * @return Full shortcut description string
   */
  static std::string formatShortcut(Key key, Modifier modifiers,
                                    const std::string &description);
};

/**
 * @brief Bitwise OR operator for combining Modifier flags
 */
inline KeyboardShortcutManager::Modifier
operator|(KeyboardShortcutManager::Modifier lhs,
          KeyboardShortcutManager::Modifier rhs) {
  return static_cast<KeyboardShortcutManager::Modifier>(static_cast<int>(lhs) |
                                                        static_cast<int>(rhs));
}

/**
 * @brief Bitwise AND operator for checking Modifier flags
 */
inline KeyboardShortcutManager::Modifier
operator&(KeyboardShortcutManager::Modifier lhs,
          KeyboardShortcutManager::Modifier rhs) {
  return static_cast<KeyboardShortcutManager::Modifier>(static_cast<int>(lhs) &
                                                        static_cast<int>(rhs));
}

} // namespace ImGUI
} // namespace view
