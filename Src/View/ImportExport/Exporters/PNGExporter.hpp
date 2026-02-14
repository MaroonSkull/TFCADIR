/**
 * @file PNGExporter.hpp
 * @brief PNG file format exporter
 *
 * Exports TFCADIR figures to PNG (Portable Network Graphics) format.
 * Renders figures to a raster image using software rendering.
 */

#pragma once

#include "View/ImportExport/ImporterExporterInterfaces.hpp"

#include <Model/FlatFigure.hpp>

#include <memory>
#include <string>
#include <vector>

namespace view {
namespace import_export {

/**
 * @brief PNG file format exporter
 *
 * Converts TFCADIR figure objects to PNG raster images.
 * Uses software rendering for portability.
 */
class PNGExporter : public IExporter {
public:
  PNGExporter();
  ~PNGExporter() override = default;

  /**
   * @brief Export figures to a PNG file
   * @param figures Vector of figures to export
   * @param filePath Path to the output PNG file
   * @return ExportResult with success status and any messages
   */
  ExportResult
  exportToFile(const std::vector<std::shared_ptr<model::IFigure>> &figures,
               const std::string &filePath) override;

  /**
   * @brief Get the file format this exporter handles
   * @return FileFormat::PNG
   */
  FileFormat getFormat() const override { return FileFormat::PNG; }

  /**
   * @brief Get the exporter name
   * @return "PNG Exporter"
   */
  std::string getName() const override { return "PNG Exporter"; }

  /**
   * @brief Check if a figure type is supported for export
   * @param figure The figure to check
   * @return true for all figure types (PNG supports all via rendering)
   */
  bool
  supportsFigure(const std::shared_ptr<model::IFigure> &figure) const override;

  /**
   * @brief Set the output image width
   * @param width Width in pixels
   */
  void setImageWidth(int width) { imageWidth_ = width; }

  /**
   * @brief Set the output image height
   * @param height Height in pixels
   */
  void setImageHeight(int height) { imageHeight_ = height; }

  /**
   * @brief Set the background color
   * @param r Red component (0-1)
   * @param g Green component (0-1)
   * @param b Blue component (0-1)
   */
  void setBackgroundColor(float r, float g, float b);

private:
  int imageWidth_{800};
  int imageHeight_{600};
  float bgColorR_{1.0f};
  float bgColorG_{1.0f};
  float bgColorB_{1.0f};

  /**
   * @brief Calculate bounding box of all figures
   * @param figures Vector of figures
   * @return Pair of min and max bounds
   */
  static std::pair<glm::vec3, glm::vec3>
  calculateBounds(const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Write PNG file using stb_image_write
   * @param filePath Output file path
   * @param pixels Pixel data (RGB)
   * @param width Image width
   * @param height Image height
   * @return true if write succeeded
   */
  static bool writePNG(const std::string &filePath, const unsigned char *pixels,
                       int width, int height);

  /**
   * @brief Draw a line using Bresenham's algorithm
   * @param pixels Pixel buffer
   * @param width Image width
   * @param height Image height
   * @param x1 Start X coordinate
   * @param y1 Start Y coordinate
   * @param x2 End X coordinate
   * @param y2 End Y coordinate
   * @param r Red color component
   * @param g Green color component
   * @param b Blue color component
   */
  static void drawLine(std::vector<unsigned char> &pixels, int width,
                       int height, int x1, int y1, int x2, int y2,
                       unsigned char r, unsigned char g, unsigned char b);

  /**
   * @brief Draw a circle using midpoint algorithm
   * @param pixels Pixel buffer
   * @param width Image width
   * @param height Image height
   * @param cx Center X coordinate
   * @param cy Center Y coordinate
   * @param radius Circle radius
   * @param r Red color component
   * @param g Green color component
   * @param b Blue color component
   */
  static void drawCircle(std::vector<unsigned char> &pixels, int width,
                         int height, int cx, int cy, int radius,
                         unsigned char r, unsigned char g, unsigned char b);

  /**
   * @brief Draw a regular n-gon
   * @param pixels Pixel buffer
   * @param width Image width
   * @param height Image height
   * @param cx Center X coordinate
   * @param cy Center Y coordinate
   * @param radius Circumradius
   * @param n Number of sides
   * @param r Red color component
   * @param g Green color component
   * @param b Blue color component
   */
  static void drawNgon(std::vector<unsigned char> &pixels, int width,
                       int height, int cx, int cy, int radius, int n,
                       unsigned char r, unsigned char g, unsigned char b);
};

} // namespace import_export
} // namespace view
