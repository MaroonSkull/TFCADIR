#include "RenderingPipeline3D.hpp"

#include <glad/gl.h>

namespace view {

/**
 * @brief Default background color (dark gray)
 */
static constexpr glm::vec3 DEFAULT_BACKGROUND_COLOR{0.15f, 0.15f, 0.15f};

RenderingPipeline3D::RenderingPipeline3D()
    : camera_(), orbitControls_(camera_), gridRenderer_(), axesRenderer_(),
      objectRenderer_(), backgroundColor_(DEFAULT_BACKGROUND_COLOR),
      viewportWidth_(0), viewportHeight_(0), initialized_(false) {}

RenderingPipeline3D::~RenderingPipeline3D() { cleanup(); }

RenderingPipeline3D::RenderingPipeline3D(RenderingPipeline3D &&other) noexcept
    : camera_(std::move(other.camera_)),
      orbitControls_(camera_), // Rebind to our camera, not moved
      gridRenderer_(std::move(other.gridRenderer_)),
      axesRenderer_(std::move(other.axesRenderer_)),
      objectRenderer_(std::move(other.objectRenderer_)),
      backgroundColor_(other.backgroundColor_),
      viewportWidth_(other.viewportWidth_),
      viewportHeight_(other.viewportHeight_), initialized_(other.initialized_) {
  other.initialized_ = false;
}

// Note: Move assignment operator is deleted because OrbitControls contains a
// reference member and cannot be assigned. Use move constructor instead.

bool RenderingPipeline3D::initialize(int viewportWidth, int viewportHeight) {
  if (initialized_) {
    return true;
  }

  viewportWidth_ = viewportWidth;
  viewportHeight_ = viewportHeight;

  // Initialize OpenGL state
  setupGLState();

  // All renderers initialize themselves in their constructors
  // The grid, axes, and object renderers are already initialized

  initialized_ = true;
  return true;
}

void RenderingPipeline3D::cleanup() {
  if (!initialized_) {
    return;
  }

  // Renderers clean up their own resources in destructors
  initialized_ = false;
}

void RenderingPipeline3D::render() {
  if (!initialized_) {
    return;
  }

  // Clear the screen
  clearScreen();

  // Calculate view and projection matrices
  float aspectRatio =
      static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
  glm::mat4 view = camera_.getViewMatrix();
  glm::mat4 projection = camera_.getProjectionMatrix(aspectRatio);

  // Render in order: grid, axes, objects
  gridRenderer_.render(view, projection);
  axesRenderer_.render(view, projection);
  objectRenderer_.render(camera_, aspectRatio);
}

void RenderingPipeline3D::onViewportResize(int width, int height) {
  viewportWidth_ = width;
  viewportHeight_ = height;

  // Update OpenGL viewport
  glViewport(0, 0, width, height);
}

void RenderingPipeline3D::setBackgroundColor(const glm::vec3 &color) {
  backgroundColor_ = color;
}

void RenderingPipeline3D::setGridVisible(bool visible) {
  gridRenderer_.setVisible(visible);
}

void RenderingPipeline3D::setAxesVisible(bool visible) {
  axesRenderer_.setVisible(visible);
}

bool RenderingPipeline3D::isGridVisible() const {
  return gridRenderer_.isVisible();
}

bool RenderingPipeline3D::isAxesVisible() const {
  return axesRenderer_.isVisible();
}

void RenderingPipeline3D::setupGLState() {
  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Enable line smoothing for better-looking lines
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  // Set up face culling (optional, can be enabled for performance)
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
}

void RenderingPipeline3D::clearScreen() {
  // Clear color and depth buffers
  glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b,
               1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace view
