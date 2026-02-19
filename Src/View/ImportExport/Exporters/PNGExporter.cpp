/**
 * @file PNGExporter.cpp
 * @brief PNG file format exporter implementation
 *
 * Note: This implementation writes PPM (Portable Pixmap) format which is
 * a simple uncompressed image format. The resulting .ppm files can be
 * easily converted to PNG using tools like ImageMagick or online converters.
 * A full PNG implementation would require a library like libpng or
 * stb_image_write.
 */

#include "PNGExporter.hpp"

// Must be defined before including <cmath> for M_PI to be available on MSVC
#define _USE_MATH_DEFINES
#include <cmath>

#include <Model/FlatFigure.hpp>

#include <algorithm>
#include <fstream>
#include <limits>
#include <vector>

namespace view {
namespace import_export {

PNGExporter::PNGExporter() = default;

ExportResult PNGExporter::exportToFile(
    const std::vector<std::shared_ptr<model::IFigure>> &figures,
    const std::string &filePath) {

  ExportResult result;

  if (figures.empty()) {
    result.success = false;
    result.errorMessage = "No figures to export";
    return result;
  }

  // Calculate bounds for proper scaling
  auto bounds = calculateBounds(figures);
  float minX = bounds.first.x;
  float minY = bounds.first.y;
  float maxX = bounds.second.x;
  float maxY = bounds.second.y;

  // Calculate scale to fit in image
  float width = maxX - minX;
  float height = maxY - minY;

  // Prevent division by zero
  if (width < 0.001f) {
    width = 100.0f;
  }
  if (height < 0.001f) {
    height = 100.0f;
  }

  float scale = std::min(static_cast<float>(imageWidth_) / width,
                         static_cast<float>(imageHeight_) / height) *
                0.9f;
  float offsetX = (imageWidth_ - width * scale) / 2.0f;
  float offsetY = (imageHeight_ - height * scale) / 2.0f;

  // Create pixel buffer (RGB)
  std::vector<unsigned char> pixels(static_cast<size_t>(imageWidth_) *
                                    static_cast<size_t>(imageHeight_) * 3);

  // Fill with background color
  for (int i = 0; i < imageWidth_ * imageHeight_; ++i) {
    pixels[static_cast<size_t>(i) * 3] =
        static_cast<unsigned char>(bgColorR_ * 255.0f);
    pixels[static_cast<size_t>(i) * 3 + 1] =
        static_cast<unsigned char>(bgColorG_ * 255.0f);
    pixels[static_cast<size_t>(i) * 3 + 2] =
        static_cast<unsigned char>(bgColorB_ * 255.0f);
  }

  // Transform world coordinates to pixel coordinates
  auto worldToPixel = [&](float x, float y) -> std::pair<int, int> {
    int px = static_cast<int>((x - minX) * scale + offsetX);
    int py = static_cast<int>((maxY - y) * scale + offsetY); // Flip Y
    return {px, py};
  };

  // Draw each figure
  for (const auto &figure : figures) {
    if (!figure) {
      continue;
    }

    auto appearance = figure->getAppearance();
    unsigned char r =
        static_cast<unsigned char>(appearance.strokeColor.r * 255.0f);
    unsigned char g =
        static_cast<unsigned char>(appearance.strokeColor.g * 255.0f);
    unsigned char b =
        static_cast<unsigned char>(appearance.strokeColor.b * 255.0f);

    // Draw based on figure type
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      auto [x1, y1] = worldToPixel(tri->first.x, tri->first.y);
      auto [x2, y2] = worldToPixel(tri->second.x, tri->second.y);
      auto [x3, y3] = worldToPixel(tri->third.x, tri->third.y);

      // Draw triangle edges
      drawLine(pixels, imageWidth_, imageHeight_, x1, y1, x2, y2, r, g, b);
      drawLine(pixels, imageWidth_, imageHeight_, x2, y2, x3, y3, r, g, b);
      drawLine(pixels, imageWidth_, imageHeight_, x3, y3, x1, y1, r, g, b);

    } else if (auto quad =
                   std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                       figure)) {
      auto [x1, y1] = worldToPixel(quad->first.x, quad->first.y);
      auto [x2, y2] = worldToPixel(quad->second.x, quad->second.y);
      auto [x3, y3] = worldToPixel(quad->third.x, quad->third.y);
      auto [x4, y4] = worldToPixel(quad->fourth.x, quad->fourth.y);

      drawLine(pixels, imageWidth_, imageHeight_, x1, y1, x2, y2, r, g, b);
      drawLine(pixels, imageWidth_, imageHeight_, x2, y2, x3, y3, r, g, b);
      drawLine(pixels, imageWidth_, imageHeight_, x3, y3, x4, y4, r, g, b);
      drawLine(pixels, imageWidth_, imageHeight_, x4, y4, x1, y1, r, g, b);

    } else if (auto circle =
                   std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                       figure)) {
      auto [cx, cy] = worldToPixel(circle->center.x, circle->center.y);
      int radius = static_cast<int>(circle->radius * scale);
      drawCircle(pixels, imageWidth_, imageHeight_, cx, cy, radius, r, g, b);

    } else if (auto ngon =
                   std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                       figure)) {
      auto [cx, cy] = worldToPixel(ngon->center.x, ngon->center.y);
      int radius = static_cast<int>(ngon->radius * scale);
      int n = static_cast<int>(ngon->n);
      if (n < 3) {
        n = 3;
      }
      drawNgon(pixels, imageWidth_, imageHeight_, cx, cy, radius, n, r, g, b);
    }
  }

  // Write PPM file (can be converted to PNG)
  // Change extension to .ppm for proper format indication
  std::string outputPath = filePath;
  if (outputPath.size() > 4 &&
      outputPath.substr(outputPath.size() - 4) == ".png") {
    outputPath = outputPath.substr(0, outputPath.size() - 4) + ".ppm";
  }

  if (!writePNG(outputPath, pixels.data(), imageWidth_, imageHeight_)) {
    result.success = false;
    result.errorMessage = "Failed to write image file: " + outputPath;
    return result;
  }

  result.success = true;
  result.figuresExported = figures.size();
  result.warnings.push_back(
      "Image saved as PPM format. Convert to PNG using: convert " + outputPath +
      " " + filePath);
  return result;
}

bool PNGExporter::supportsFigure(
    const std::shared_ptr<model::IFigure> &figure) const {
  // PNG supports all figure types via rendering
  return figure != nullptr;
}

void PNGExporter::setBackgroundColor(float r, float g, float b) {
  bgColorR_ = std::clamp(r, 0.0f, 1.0f);
  bgColorG_ = std::clamp(g, 0.0f, 1.0f);
  bgColorB_ = std::clamp(b, 0.0f, 1.0f);
}

std::pair<glm::vec3, glm::vec3> PNGExporter::calculateBounds(
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {

  glm::vec3 minBounds(std::numeric_limits<float>::max());
  glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

  for (const auto &figure : figures) {
    if (!figure) {
      continue;
    }

    auto bounds = figure->getBounds();
    minBounds = glm::min(minBounds, bounds.first);
    maxBounds = glm::max(maxBounds, bounds.second);
  }

  // Handle empty figures case
  if (minBounds.x == std::numeric_limits<float>::max()) {
    return {glm::vec3(0.0f), glm::vec3(100.0f)};
  }

  return {minBounds, maxBounds};
}

bool PNGExporter::writePNG(const std::string &filePath,
                           const unsigned char *pixels, int width, int height) {
  // Write PPM format (P6 - binary RGB)
  std::ofstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  // PPM header
  file << "P6\n" << width << " " << height << "\n255\n";

  // Write pixel data
  file.write(reinterpret_cast<const char *>(pixels),
             static_cast<std::streamsize>(width) * height * 3);

  file.close();
  return true;
}

void PNGExporter::drawLine(std::vector<unsigned char> &pixels, int width,
                           int height, int x1, int y1, int x2, int y2,
                           unsigned char r, unsigned char g, unsigned char b) {
  // Bresenham's line algorithm
  int dx = std::abs(x2 - x1);
  int dy = std::abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1;
  int sy = y1 < y2 ? 1 : -1;
  int err = dx - dy;

  while (true) {
    // Set pixel if within bounds
    if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
      size_t idx = static_cast<size_t>((y1 * width + x1) * 3);
      pixels[idx] = r;
      pixels[idx + 1] = g;
      pixels[idx + 2] = b;
    }

    if (x1 == x2 && y1 == y2) {
      break;
    }

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
}

void PNGExporter::drawCircle(std::vector<unsigned char> &pixels, int width,
                             int height, int cx, int cy, int radius,
                             unsigned char r, unsigned char g,
                             unsigned char b) {
  // Midpoint circle algorithm
  int x = radius;
  int y = 0;
  int err = 0;

  while (x >= y) {
    // Draw 8 octants
    auto setPixel = [&](int px, int py) {
      if (px >= 0 && px < width && py >= 0 && py < height) {
        size_t idx = static_cast<size_t>((py * width + px) * 3);
        pixels[idx] = r;
        pixels[idx + 1] = g;
        pixels[idx + 2] = b;
      }
    };

    setPixel(cx + x, cy + y);
    setPixel(cx + y, cy + x);
    setPixel(cx - y, cy + x);
    setPixel(cx - x, cy + y);
    setPixel(cx - x, cy - y);
    setPixel(cx - y, cy - x);
    setPixel(cx + y, cy - x);
    setPixel(cx + x, cy - y);

    y++;
    if (err <= 0) {
      err += 2 * y + 1;
    }
    if (err > 0) {
      x--;
      err -= 2 * x + 1;
    }
  }
}

void PNGExporter::drawNgon(std::vector<unsigned char> &pixels, int width,
                           int height, int cx, int cy, int radius, int n,
                           unsigned char r, unsigned char g, unsigned char b) {
  if (n < 3) {
    return;
  }

  // Calculate vertices
  std::vector<std::pair<int, int>> vertices;
  for (int i = 0; i < n; ++i) {
    float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) /
                      static_cast<float>(n) -
                  static_cast<float>(M_PI) / 2.0f; // Start from top
    int x = cx + static_cast<int>(static_cast<float>(radius) * std::cos(angle));
    int y = cy + static_cast<int>(static_cast<float>(radius) * std::sin(angle));
    vertices.emplace_back(x, y);
  }

  // Draw edges
  for (int i = 0; i < n; ++i) {
    int next = (i + 1) % n;
    drawLine(pixels, width, height, vertices[static_cast<size_t>(i)].first,
             vertices[static_cast<size_t>(i)].second,
             vertices[static_cast<size_t>(next)].first,
             vertices[static_cast<size_t>(next)].second, r, g, b);
  }
}

} // namespace import_export
} // namespace view
