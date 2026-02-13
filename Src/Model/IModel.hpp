#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace model {

/**
 * @brief Appearance properties for a figure
 * @details Stores stroke and fill colors, line width, and visibility flags
 */
struct Appearance {
  glm::vec3 strokeColor{1.0f, 1.0f, 1.0f}; ///< Stroke color (RGB, 0-1 range)
  glm::vec3 fillColor{0.5f, 0.5f, 0.5f};   ///< Fill color (RGB, 0-1 range)
  float lineWidth{1.0f};                   ///< Line width in pixels
  bool strokeEnabled{true};                ///< Whether stroke is visible
  bool fillEnabled{true};                  ///< Whether fill is visible
};

/**
 * @brief Layer properties for a figure
 * @details Stores layer assignment, visibility, and lock state
 */
struct LayerProperties {
  int layerIndex{0};  ///< Layer index (0 = default layer)
  bool visible{true}; ///< Whether figure is visible
  bool locked{false}; ///< Whether figure is locked (non-editable)
};

/**
 * @brief Abstract base interface for all figure types
 * @details Provides a common abstraction for geometric entities with position
 *          and identification capabilities. This interface follows the
 * Interface Segregation Principle by providing focused, cohesive methods.
 */
class IFigure {
public:
  virtual ~IFigure() = default;

  /**
   * @brief Gets the unique identifier for this figure
   * @return The figure's unique ID
   */
  virtual uint32_t getId() const = 0;

  /**
   * @brief Gets the name of this figure
   * @return The figure's name
   */
  virtual const std::string &getName() const = 0;

  /**
   * @brief Sets the name of this figure
   * @param name The new name for the figure
   */
  virtual void setName(const std::string &name) = 0;

  /**
   * @brief Gets the position of this figure in 3D space
   * @return The figure's position as a 3D vector
   * @note For 2D figures, the z-component will be 0
   */
  virtual glm::vec3 getPosition() const = 0;

  /**
   * @brief Sets the position of this figure in 3D space
   * @param position The new position for the figure
   */
  virtual void setPosition(const glm::vec3 &position) = 0;

  /**
   * @brief Gets the bounding box of this figure
   * @return A pair of vec3 representing min and max bounds
   * @note The first element is the minimum corner, second is the maximum corner
   */
  virtual std::pair<glm::vec3, glm::vec3> getBounds() const = 0;

  // === Appearance Properties ===

  /**
   * @brief Gets the stroke color of this figure
   * @return Stroke color as RGB vector (0-1 range)
   */
  virtual glm::vec3 getStrokeColor() const = 0;

  /**
   * @brief Sets the stroke color of this figure
   * @param color Stroke color as RGB vector (0-1 range)
   */
  virtual void setStrokeColor(const glm::vec3 &color) = 0;

  /**
   * @brief Gets the fill color of this figure
   * @return Fill color as RGB vector (0-1 range)
   */
  virtual glm::vec3 getFillColor() const = 0;

  /**
   * @brief Sets the fill color of this figure
   * @param color Fill color as RGB vector (0-1 range)
   */
  virtual void setFillColor(const glm::vec3 &color) = 0;

  /**
   * @brief Gets the appearance properties of this figure
   * @return Appearance struct with all appearance settings
   */
  virtual Appearance getAppearance() const = 0;

  /**
   * @brief Sets the appearance properties of this figure
   * @param appearance Appearance struct with all appearance settings
   */
  virtual void setAppearance(const Appearance &appearance) = 0;

  // === Layer Properties ===

  /**
   * @brief Gets the layer index of this figure
   * @return Layer index (0 = default layer)
   */
  virtual int getLayer() const = 0;

  /**
   * @brief Sets the layer index of this figure
   * @param layerIndex Layer index (0 = default layer)
   */
  virtual void setLayer(int layerIndex) = 0;

  /**
   * @brief Checks if this figure is visible
   * @return true if visible, false if hidden
   */
  virtual bool isVisible() const = 0;

  /**
   * @brief Sets the visibility of this figure
   * @param visible true to show, false to hide
   */
  virtual void setVisible(bool visible) = 0;

  /**
   * @brief Checks if this figure is locked
   * @return true if locked (non-editable), false if editable
   */
  virtual bool isLocked() const = 0;

  /**
   * @brief Sets the lock state of this figure
   * @param locked true to lock, false to unlock
   */
  virtual void setLocked(bool locked) = 0;

  /**
   * @brief Gets the layer properties of this figure
   * @return LayerProperties struct with all layer settings
   */
  virtual LayerProperties getLayerProperties() const = 0;

  /**
   * @brief Sets the layer properties of this figure
   * @param props LayerProperties struct with all layer settings
   */
  virtual void setLayerProperties(const LayerProperties &props) = 0;

  // === Creation Date ===

  /**
   * @brief Gets the creation timestamp of this figure
   * @return Creation time as ISO 8601 string
   */
  virtual std::string getCreationDate() const = 0;
};

/**
 * @brief Abstract camera interface for view transformations
 * @details Provides camera operations for both 2D and 3D rendering.
 *          Uses glm::vec3 for all vectors to support 3D cameras while
 *          still working for 2D (z-component can be 0 or -1 for position).
 */
class ICamera {
public:
  virtual ~ICamera() = default;

  /**
   * @brief Sets the camera position in 3D space
   * @param position The new camera position
   */
  virtual void setPosition(const glm::vec3 &position) = 0;

  /**
   * @brief Gets the current camera position
   * @return The current camera position
   */
  virtual glm::vec3 getPosition() const = 0;

  /**
   * @brief Sets the camera target (look-at point)
   * @param target The point the camera should look at
   */
  virtual void setTarget(const glm::vec3 &target) = 0;

  /**
   * @brief Gets the current camera target
   * @return The current target position
   */
  virtual glm::vec3 getTarget() const = 0;

  /**
   * @brief Sets the camera up vector (orientation)
   * @param up The up vector defining camera orientation
   * @note This is essential for 3D cameras to define which direction is "up"
   */
  virtual void setUp(const glm::vec3 &up) = 0;

  /**
   * @brief Gets the current camera up vector
   * @return The current up vector
   */
  virtual glm::vec3 getUp() const = 0;
};

/**
 * @brief Abstract model interface for entity management
 * @details Provides a pure abstract interface for model implementations
 *          supporting both 2D and 3D entities. Uses glm::vec3 for all
 *          coordinates to ensure compatibility with 3D systems while
 *          still supporting 2D (z=0).
 * @note This interface serves as the Model component in the MVC architecture
 */
class IModel {
public:
  virtual ~IModel() = default;

  /**
   * @brief Adds a new figure/entity to the model
   * @param position The position where the figure should be created
   * @note The actual figure type is determined by the concrete implementation
   */
  virtual void addFigure(const glm::vec3 &position) = 0;

  /**
   * @brief Removes a figure/entity from the model by its identifier
   * @param id The unique identifier of the figure to remove
   * @return true if the figure was found and removed, false otherwise
   */
  virtual bool removeFigure(uint32_t id) = 0;

  /**
   * @brief Retrieves a figure/entity by its identifier
   * @param id The unique identifier of the figure to retrieve
   * @return Pointer to the figure if found, or nullptr if not found
   * @note The return type uses a shared_ptr to support polymorphic figure types
   *       and ensure proper lifetime management
   */
  virtual std::shared_ptr<IFigure> getFigure(uint32_t id) = 0;

  /**
   * @brief Finds a figure near the specified coordinates
   * @param coords The coordinates to search around (using x and y components)
   * @param delta Maximum distance to consider a figure as "found"
   * @return Pointer to the found figure if any, or nullptr if none found
   * @note The search uses the x and y components of coords for compatibility
   *       with 2D figures
   */
  virtual std::shared_ptr<IFigure> findFigureByCoords(const glm::vec3 &coords,
                                                      float delta) = 0;

  /**
   * @brief Updates the position of an existing figure
   * @param id The unique identifier of the figure to update
   * @param newPosition The new position for the figure
   * @return true if the figure was found and updated, false otherwise
   */
  virtual bool updateFigurePosition(uint32_t id,
                                    const glm::vec3 &newPosition) = 0;

  /**
   * @brief Gets the total number of figures in the model
   * @return The count of figures currently stored
   */
  virtual size_t getFigureCount() const = 0;

  /**
   * @brief Clears all figures from the model
   */
  virtual void clearFigures() = 0;

  /**
   * @brief Gets the camera interface for this model
   * @return Reference to the camera interface
   * @note Returns a reference to ensure the camera is always valid
   *       and ownership is clear (model owns the camera)
   */
  virtual ICamera &getCamera() = 0;

  /**
   * @brief Gets the camera interface for this model (const version)
   * @return Const reference to the camera interface
   */
  virtual const ICamera &getCamera() const = 0;

  /**
   * @brief Gets the model transformation matrix
   * @return The current model matrix
   */
  virtual glm::mat4 getModelMatrix() const = 0;

  /**
   * @brief Sets the model transformation matrix
   * @param matrix The new model matrix
   */
  virtual void setModelMatrix(const glm::mat4 &matrix) = 0;

  /**
   * @brief Gets the view transformation matrix
   * @return The current view matrix
   */
  virtual glm::mat4 getViewMatrix() const = 0;

  /**
   * @brief Gets the projection transformation matrix
   * @return The current projection matrix
   */
  virtual glm::mat4 getProjectionMatrix() const = 0;

  /**
   * @brief Sets the projection transformation matrix
   * @param matrix The new projection matrix
   */
  virtual void setProjectionMatrix(const glm::mat4 &matrix) = 0;
};

} // namespace model
