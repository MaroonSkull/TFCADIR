#pragma once

#include <glad/gl.h>
#include <Shader.hpp>
#include <set>
#include <optional>

class GLProgram {
private:
	// множество шейдеров, из которых будем собирать программу
	std::set<const Shader*, Shader::cmp> shadersList_;
	GLuint programId_{};

	void checkProgram(GLenum pname, std::string_view msg) const;
public:
	GLProgram();
	~GLProgram();

	void attachShader(const Shader*);
	void linkProgram() const;
	void useProgram() const;

	std::optional<const Shader *>getShader(Shader::type) const;
};