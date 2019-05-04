#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

namespace
{
	const std::string SHADER_PATH = "shaders/";
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
	// Get the vertex and fragment shaders' source code from the file path.
	std::string vertexCode, fragmentCode;
	std::ifstream vShaderFile, fShaderFile;

	// Ensure ifstream objects can throw exceptions.
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open the shader files.
		vShaderFile.open(SHADER_PATH + vertexPath);
		fShaderFile.open(SHADER_PATH + fragmentPath);

		// Read file’s buffer contents into streams.
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		// Close file handlers.
		vShaderFile.close();
		fShaderFile.close();

		// Convert stream into string.
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Shader file could not be read." << std::endl;
	}

	const char *vShaderCode{ vertexCode.c_str() };
	const char *fShaderCode{ fragmentCode.c_str() };

	// Compile the shaders.
	GLuint vertexId, fragmentId;
	int success;
	char infoLog[512];

	// Compile vertex shader.
	vertexId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexId, 1, &vShaderCode, NULL);
	glCompileShader(vertexId);

	// Print any compile errors.
	glGetShaderiv(vertexId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexId, 512, NULL, infoLog);
		std::cout << "Vertex shader failed to compile.\n" << infoLog << std::endl;
	};

	// Compile fragment shader.
	fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentId, 1, &fShaderCode, NULL);
	glCompileShader(fragmentId);

	// Print any compile errors.
	glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentId, 512, NULL, infoLog);
		std::cout << "Fragment shader failed to compile.\n" << infoLog << std::endl;
	};

	// Create and link the shader program with the vertex and fragment shaders.
	m_id = glCreateProgram();
	glAttachShader(m_id, vertexId);
	glAttachShader(m_id, fragmentId);
	glLinkProgram(m_id);

	// Print any linking errors.
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_id, 512, NULL, infoLog);
		std::cout << "Program failed to link.\n" << infoLog << std::endl;
	}

	// Delete the shader objects after linking.
	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	std::cout << "Shader created: " << vertexPath << ", " << fragmentPath << std::endl;
}

Shader::~Shader()
{
	glDeleteProgram(m_id);
}

void Shader::use()
{
	glUseProgram(m_id);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}