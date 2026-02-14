#pragma once

#include <Shader.hpp>

#include <optional>
#include <set>

class GLProgram {
private:
  // set of shaders from which the program will be built
  std::set<const Shader *, Shader::cmp> shadersList_;
  GLuint programId_{};

  void checkProgram(GLenum pname, std::string_view msg) const;

public:
  GLProgram();
  ~GLProgram();

  void attachShader(const Shader *);
  void linkProgram() const;
  void useProgram() const;

  std::optional<const Shader *> getShader(Shader::type) const;
  std::optional<GLint> getUniformLocation(std::string_view varName) const;
};