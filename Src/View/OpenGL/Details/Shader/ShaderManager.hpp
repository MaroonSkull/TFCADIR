#pragma once

#include "ShaderLoader.hpp"

#include <GLProgram.hpp>
#include <Shader.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace view {

/**
 * @brief Result of a shader program linking operation
 * @details Contains the linked program and any error information
 */
struct ShaderProgramResult {
  /// Linked shader program (nullptr if linking failed)
  std::unique_ptr<GLProgram> program;

  /// Error message if linking failed
  std::string errorMessage;

  /// Indicates whether linking was successful
  bool success{false};
};

/**
 * @brief Manages shader programs with caching
 * @details Provides functionality to load, compile, and link shader programs.
 *          Caches compiled shaders and linked programs for efficient reuse.
 *          Integrates with the existing GLProgram and Shader classes.
 */
class ShaderManager {
public:
  /**
   * @brief Constructs a ShaderManager with empty caches
   */
  ShaderManager() = default;

  /**
   * @brief Destructor - cleans up cached shaders and programs
   */
  ~ShaderManager() = default;

  // Prevent copying
  ShaderManager(const ShaderManager &) = delete;
  ShaderManager &operator=(const ShaderManager &) = delete;

  // Allow moving
  ShaderManager(ShaderManager &&) noexcept = default;
  ShaderManager &operator=(ShaderManager &&) noexcept = default;

  /**
   * @brief Loads and compiles a vertex shader from a file
   * @param name Unique name for the shader (used for caching)
   * @param filepath Path to the vertex shader source file
   * @return true if successful, false otherwise
   * @details Loads the shader source from file, compiles it, and caches the
   * result.
   */
  bool loadVertexShader(const std::string &name,
                        const std::filesystem::path &filepath) {
    std::string errorMessage;
    auto source = ShaderLoader::loadFromFile(filepath, errorMessage);
    if (!source) {
      lastError_ = errorMessage;
      return false;
    }

    return compileAndCacheShader(name, *source, Shader::type::Vertex);
  }

  /**
   * @brief Loads and compiles a fragment shader from a file
   * @param name Unique name for the shader (used for caching)
   * @param filepath Path to the fragment shader source file
   * @return true if successful, false otherwise
   */
  bool loadFragmentShader(const std::string &name,
                           const std::filesystem::path &filepath) {
    std::string errorMessage;
    auto source = ShaderLoader::loadFromFile(filepath, errorMessage);
    if (!source) {
      lastError_ = errorMessage;
      return false;
    }

    return compileAndCacheShader(name, *source, Shader::type::Fragment);
  }

  /**
   * @brief Loads and compiles a geometry shader from a file
   * @param name Unique name for the shader (used for caching)
   * @param filepath Path to the geometry shader source file
   * @return true if successful, false otherwise
   */
  bool loadGeometryShader(const std::string &name,
                           const std::filesystem::path &filepath) {
    std::string errorMessage;
    auto source = ShaderLoader::loadFromFile(filepath, errorMessage);
    if (!source) {
      lastError_ = errorMessage;
      return false;
    }

    return compileAndCacheShader(name, *source, Shader::type::Geometry);
  }

  /**
   * @brief Compiles and caches a shader from source code
   * @param name Unique name for the shader
   * @param source The shader source code
   * @param shaderType The type of shader (Vertex, Fragment, etc.)
   * @return true if successful, false otherwise
   */
  bool compileAndCacheShader(const std::string &name, const std::string &source,
                              Shader::type shaderType) {
    // Create a Resource wrapper for the source
    Resource resource(source.data(), source.size());

    try {
      auto shader = std::make_unique<Shader>(resource, shaderType);
      shaderCache_[name] = std::move(shader);
      return true;
    } catch (const std::exception &e) {
      lastError_ = "Shader compilation failed for '" + name + "': " + e.what();
      return false;
    }
  }

  /**
   * @brief Creates a shader program from cached vertex and fragment shaders
   * @param programName Unique name for the program (used for caching)
   * @param vertexShaderName Name of the cached vertex shader
   * @param fragmentShaderName Name of the cached fragment shader
   * @return ShaderProgramResult containing the program and any error
   * information
   * @note The returned program is owned by the cache. Callers should use
   *       getProgram() to retrieve the program later, or use the returned
   *       pointer before calling clearCache() or removeProgram().
   */
  ShaderProgramResult createProgram(const std::string &programName,
                                    const std::string &vertexShaderName,
                                    const std::string &fragmentShaderName) {
    ShaderProgramResult result;

    // Find shaders in cache
    auto vertexIt = shaderCache_.find(vertexShaderName);
    auto fragmentIt = shaderCache_.find(fragmentShaderName);

    if (vertexIt == shaderCache_.end()) {
      result.errorMessage =
          "Vertex shader not found in cache: " + vertexShaderName;
      return result;
    }

    if (fragmentIt == shaderCache_.end()) {
      result.errorMessage =
          "Fragment shader not found in cache: " + fragmentShaderName;
      return result;
    }

    // Create program
    auto program = std::make_unique<GLProgram>();
    program->attachShader(vertexIt->second.get());
    program->attachShader(fragmentIt->second.get());

    try {
      program->linkProgram();
    } catch (const std::exception &e) {
      result.errorMessage =
          "Program linking failed for '" + programName + "': " + e.what();
      return result;
    }

    result.success = true;
    programCache_[programName] = std::move(program);

    return result;
  }

  /**
   * @brief Gets a cached shader program by name
   * @param name The name of the program to retrieve
   * @return Pointer to the program, or nullptr if not found
   * @note The returned pointer is valid only while the program remains in
   *       the cache. Do not call clearCache() or removeProgram() while
   *       using the returned pointer.
   */
  [[nodiscard]] GLProgram *getProgram(const std::string &name) {
    auto it = programCache_.find(name);
    if (it != programCache_.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  /**
   * @brief Gets a cached shader program by name (const version)
   * @param name The name of the program to retrieve
   * @return Const pointer to the program, or nullptr if not found
   */
  [[nodiscard]] const GLProgram *getProgram(const std::string &name) const {
    auto it = programCache_.find(name);
    if (it != programCache_.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  /**
   * @brief Checks if a shader exists in the cache
   * @param name The name of the shader to check
   * @return true if the shader exists, false otherwise
   */
  [[nodiscard]] bool hasShader(const std::string &name) const {
    return shaderCache_.find(name) != shaderCache_.end();
  }

  /**
   * @brief Checks if a program exists in the cache
   * @param name The name of the program to check
   * @return true if the program exists, false otherwise
   */
  [[nodiscard]] bool hasProgram(const std::string &name) const {
    return programCache_.find(name) != programCache_.end();
  }

  /**
   * @brief Gets the last error message
   * @return The last error message, or empty string if no error
   */
  [[nodiscard]] std::string getLastError() const { return lastError_; }

  /**
   * @brief Clears all cached shaders and programs
   */
  void clearCache() {
    shaderCache_.clear();
    programCache_.clear();
    lastError_.clear();
  }

  /**
   * @brief Removes a specific shader from the cache
   * @param name The name of the shader to remove
   * @return true if the shader was removed, false if not found
   */
  bool removeShader(const std::string &name) {
    auto it = shaderCache_.find(name);
    if (it != shaderCache_.end()) {
      shaderCache_.erase(it);
      return true;
    }
    return false;
  }

  /**
   * @brief Removes a specific program from the cache
   * @param name The name of the program to remove
   * @return true if the program was removed, false if not found
   */
  bool removeProgram(const std::string &name) {
    auto it = programCache_.find(name);
    if (it != programCache_.end()) {
      programCache_.erase(it);
      return true;
    }
    return false;
  }

private:
  /// Cache of compiled shaders indexed by name
  std::unordered_map<std::string, std::unique_ptr<Shader>> shaderCache_;

  /// Cache of linked programs indexed by name
  std::unordered_map<std::string, std::unique_ptr<GLProgram>> programCache_;

  /// Last error message
  std::string lastError_;
};

} // namespace view
