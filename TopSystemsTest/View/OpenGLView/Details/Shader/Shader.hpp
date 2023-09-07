#pragma once

#include <glad/gl.h>
#include <Resource.h>
#include <string>
#include <string_view>
#include <array>

class Shader {
public:
	enum type {
		Vertex = GL_VERTEX_SHADER,
		TessEvaluation = GL_TESS_EVALUATION_SHADER,
		TessControl = GL_TESS_CONTROL_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Compute = GL_COMPUTE_SHADER
	};
private:
	Shader::type shaderType_ = Shader::type::Vertex; // Default value
	const std::string source_;
	GLuint shaderId_{};
	bool isShaderDeleted_{ false };

public:
	Shader(const Resource& Source, Shader::type);
	~Shader();

	struct cmp {
		bool operator()(const Shader*, const Shader*) const;
	};

	auto getShaderId() const -> decltype(shaderId_);
	auto getShaderType() const -> decltype(shaderType_);
	void deleteShader();

	void setUniform(std::string_view, GLfloat) const;
	void setUniform(std::string_view, const std::array<GLfloat, 4>&) const;
	void setUniform(std::string_view, bool) const;
};