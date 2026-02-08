/**
 * @file KeyboardShortcutManager.cpp
 * @brief Implementation of KeyboardShortcutManager for Phase 2 drawing tools
 *
 * This file implements the KeyboardShortcutManager class which handles keyboard
 * shortcuts for tool activation and common operations. All shortcuts use Ctrl+
 * modifier to prevent conflicts with single-letter text input.
 */

#include "KeyboardShortcutManager.hpp"
#include <imgui.h>
#include <stdexcept>

namespace view {
namespace ImGUI {

void KeyboardShortcutManager::registerShortcut(Key key, Modifier modifiers,
                                               ShortcutCallback callback,
                                               const std::string &description) {
  /// Validate that at least Ctrl+ modifier is present
  if (modifiers == Modifier::None) {
    throw std::invalid_argument(
        "Shortcuts MUST use at least Ctrl+ modifier to prevent conflicts "
        "with text input");
  }

  /// Create the shortcut registration
  Shortcut shortcut;
  shortcut.key = key;
  shortcut.modifiers = modifiers;
  shortcut.callback = std::move(callback);
  shortcut.description = description;

  /// Store the shortcut in the map
  auto keyPair = std::make_pair(key, modifiers);
  shortcuts_[keyPair] = std::move(shortcut);
}

void KeyboardShortcutManager::unregisterShortcut(Key key, Modifier modifiers) {
  /// Remove the shortcut from the map
  auto keyPair = std::make_pair(key, modifiers);
  shortcuts_.erase(keyPair);
}

void KeyboardShortcutManager::handleShortcut(const ImGuiIO &io) {
  /// Check each registered shortcut against the current input state
  for (const auto &entry : shortcuts_) {
    const Shortcut &shortcut = entry.second;

    /// Check if the shortcut is currently pressed
    if (isShortcutPressed(io, shortcut)) {
      /// Invoke the callback
      if (shortcut.callback) {
        shortcut.callback();
      }
      /// Only trigger one shortcut per frame to prevent conflicts
      break;
    }
  }
}

void KeyboardShortcutManager::clear() {
  /// Clear all registered shortcuts
  shortcuts_.clear();
}

std::vector<std::string>
KeyboardShortcutManager::getShortcutDescriptions() const {
  std::vector<std::string> descriptions;
  descriptions.reserve(shortcuts_.size());

  /// Build description strings for all shortcuts
  for (const auto &entry : shortcuts_) {
    const Shortcut &shortcut = entry.second;
    descriptions.push_back(
        formatShortcut(shortcut.key, shortcut.modifiers, shortcut.description));
  }

  return descriptions;
}

bool KeyboardShortcutManager::isShortcutPressed(
    const ImGuiIO &io, const Shortcut &shortcut) const {
  /// Check if the primary key is pressed
  bool keyPressed = io.KeysDown[static_cast<int>(shortcut.key)];
  if (!keyPressed) {
    return false;
  }

  /// Check if all required modifiers are pressed
  bool ctrlPressed = (shortcut.modifiers & Modifier::Ctrl) != Modifier::None;
  bool shiftPressed = (shortcut.modifiers & Modifier::Shift) != Modifier::None;
  bool altPressed = (shortcut.modifiers & Modifier::Alt) != Modifier::None;

  /// Verify the modifier state matches
  if (ctrlPressed != io.KeyCtrl) {
    return false;
  }
  if (shiftPressed != io.KeyShift) {
    return false;
  }
  if (altPressed != io.KeyAlt) {
    return false;
  }

  /// Check that no OTHER modifiers are pressed (unless specified)
  /// For example, if we want Ctrl+L, we should NOT accept Ctrl+Shift+L
  if (!ctrlPressed && io.KeyCtrl) {
    return false;
  }
  if (!shiftPressed && io.KeyShift) {
    return false;
  }
  if (!altPressed && io.KeyAlt) {
    return false;
  }

  return true;
}

std::string KeyboardShortcutManager::keyToString(Key key) {
  /// Convert key enum to human-readable string
  switch (key) {
  case Key::None:
    return "None";
  case Key::A:
  case Key::B:
  case Key::C:
  case Key::D:
  case Key::E:
  case Key::F:
  case Key::G:
  case Key::H:
  case Key::I:
  case Key::J:
  case Key::K:
  case Key::L:
  case Key::M:
  case Key::N:
  case Key::O:
  case Key::P:
  case Key::Q:
  case Key::R:
  case Key::S:
  case Key::T:
  case Key::U:
  case Key::V:
  case Key::W:
  case Key::X:
  case Key::Y:
  case Key::Z:
    return std::string(1, static_cast<char>(key));
  case Key::Num0:
    return "0";
  case Key::Num1:
    return "1";
  case Key::Num2:
    return "2";
  case Key::Num3:
    return "3";
  case Key::Num4:
    return "4";
  case Key::Num5:
    return "5";
  case Key::Num6:
    return "6";
  case Key::Num7:
    return "7";
  case Key::Num8:
    return "8";
  case Key::Num9:
    return "9";
  case Key::F1:
    return "F1";
  case Key::F2:
    return "F2";
  case Key::F3:
    return "F3";
  case Key::F4:
    return "F4";
  case Key::F5:
    return "F5";
  case Key::F6:
    return "F6";
  case Key::F7:
    return "F7";
  case Key::F8:
    return "F8";
  case Key::F9:
    return "F9";
  case Key::F10:
    return "F10";
  case Key::F11:
    return "F11";
  case Key::F12:
    return "F12";
  case Key::Space:
    return "Space";
  case Key::Enter:
    return "Enter";
  case Key::Escape:
    return "Escape";
  case Key::Backspace:
    return "Backspace";
  case Key::Tab:
    return "Tab";
  case Key::Insert:
    return "Insert";
  case Key::Delete:
    return "Delete";
  case Key::Home:
    return "Home";
  case Key::End:
    return "End";
  case Key::PageUp:
    return "PageUp";
  case Key::PageDown:
    return "PageDown";
  case Key::ArrowUp:
    return "Up";
  case Key::ArrowDown:
    return "Down";
  case Key::ArrowLeft:
    return "Left";
  case Key::ArrowRight:
    return "Right";
  default:
    return "Unknown";
  }
}

std::string KeyboardShortcutManager::modifiersToString(Modifier modifiers) {
  /// Convert modifier flags to human-readable string
  std::string result;

  if ((modifiers & Modifier::Ctrl) != Modifier::None) {
    result += "Ctrl+";
  }
  if ((modifiers & Modifier::Shift) != Modifier::None) {
    result += "Shift+";
  }
  if ((modifiers & Modifier::Alt) != Modifier::None) {
    result += "Alt+";
  }

  return result;
}

std::string
KeyboardShortcutManager::formatShortcut(Key key, Modifier modifiers,
                                        const std::string &description) {
  /// Combine key, modifiers, and description into a readable format
  std::string shortcutStr = modifiersToString(modifiers);
  shortcutStr += keyToString(key);
  shortcutStr += ": ";
  shortcutStr += description;
  return shortcutStr;
}

} // namespace ImGUI
} // namespace view
