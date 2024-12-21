#include <Shader.hpp>

#include <sstream>

Shader::Shader(const Resource& ShaderSource, Shader::type shaderType) : shaderType_(shaderType), source_(ShaderSource.toString()) {
	// Create shader
	shaderId_ = glCreateShader(shaderType_);

	// Compile shader source code
	const char* source = source_.c_str();
	glShaderSource(shaderId_, 1, &source, nullptr);
	glCompileShader(shaderId_);

	// Check shader compile errors
	int32_t success;
	char infoLog[512];
	glGetShaderiv(shaderId_, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderId_, 512, nullptr, infoLog);
		std::stringstream err;
		err << static_cast<int>(shaderType_)
			<< " shader compilation failed: " << std::endl
			<< infoLog;
		throw std::runtime_error{ err.str() };
	}
}


Shader::~Shader() {
	deleteShader();
}

bool Shader::cmp::operator()(const Shader* a, const Shader* b) const {
	return a->shaderType_ < b->shaderType_;
}

auto Shader::getShaderId() const -> decltype(shaderId_) {
	return shaderId_;
}

auto Shader::getShaderType() const -> decltype(shaderType_) {
	return shaderType_;
}

void Shader::deleteShader()
{
	if (isShaderDeleted_)
		return;
	glDeleteShader(shaderId_);
	isShaderDeleted_ = true;
}

void Shader::setUniform(std::string_view name, GLfloat value) const {
	glUniform1f(glGetUniformLocation(shaderId_, name.data()),
		value);
}

void Shader::setUniform(std::string_view name, const std::array<GLfloat, 4>& value) const {
	glUniform4f(glGetUniformLocation(shaderId_, name.data()),
		value.at(0), value.at(1), value.at(2), value.at(3));
}

void Shader::setUniform(std::string_view name, bool value) const {
	glUniform1i(glGetUniformLocation(shaderId_, name.data()),
		static_cast<GLint>(value));
}