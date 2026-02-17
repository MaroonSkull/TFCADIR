#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace view {

/**
 * @brief Loads shader source code from files
 * @details Provides functionality to load vertex and fragment shader source
 *          code from disk. Handles file I/O errors and provides the shader
 *          source as a string for compilation.
 */
class ShaderLoader {
public:
  /**
   * @brief Loads shader source code from a file
   * @param filepath Path to the shader source file
   * @return Optional containing the shader source code if successful,
   *         std::nullopt otherwise
   * @details Reads the entire file content as a string. Returns std::nullopt
   *          if the file cannot be opened, read, or is empty.
   */
  [[nodiscard]] static std::optional<std::string>
  loadFromFile(const std::filesystem::path &filepath) {
    if (!std::filesystem::exists(filepath)) {
      return std::nullopt;
    }

    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
      return std::nullopt;
    }

    // Read entire file content
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Check for empty file
    if (content.empty()) {
      return std::nullopt;
    }

    return content;
  }

  /**
   * @brief Loads shader source code from a file with error message
   * @param filepath Path to the shader source file
   * @param errorMessage Output parameter for error message if loading fails
   * @return Optional containing the shader source code if successful,
   *         std::nullopt otherwise
   * @details Reads the entire file content as a string. Provides detailed
   *          error messages for debugging purposes.
   */
  [[nodiscard]] static std::optional<std::string>
  loadFromFile(const std::filesystem::path &filepath,
               std::string &errorMessage) {
    if (!std::filesystem::exists(filepath)) {
      errorMessage = "Shader file does not exist: " + filepath.string();
      return std::nullopt;
    }

    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
      errorMessage = "Failed to open shader file: " + filepath.string();
      return std::nullopt;
    }

    // Read entire file content
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    if (content.empty()) {
      errorMessage = "Shader file is empty: " + filepath.string();
      return std::nullopt;
    }

    return content;
  }
};

} // namespace view
