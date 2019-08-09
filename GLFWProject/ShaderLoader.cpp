#include "ShaderLoader.h"

#include "Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <sstream>

namespace
{
	const int NUM_STREAMS_REQUIRED{ 2 };
}

ShaderLoader::ShaderLoader()
{
	m_numStreamsRequired = NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType> ShaderLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	auto vShaderFile{ streams[0].stream };
	auto fShaderFile{ streams[1].stream };

	// Get the vertex and fragment shaders' source code from the file path.
	std::string vertexCode, fragmentCode;

	// Ensure ifstream objects can throw exceptions.
	vShaderFile->exceptions(std::fstream::failbit | std::fstream::badbit);
	fShaderFile->exceptions(std::fstream::failbit | std::fstream::badbit);

	try
	{
		// Read file’s buffer contents into streams.
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile->rdbuf();
		fShaderStream << fShaderFile->rdbuf();

		// Convert stream into string.
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ShaderLoader::loadFromStream: Shader file '" << name << 
			"' could not be read." << std::endl;
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
		std::cout << "ShaderLoader::loadFromStream: Vertex shader failed to compile for '" <<
			name << "'\n" << infoLog << std::endl;
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
		std::cout << "ShaderLoader::loadFromStream: Fragment shader failed to compile for '" <<
			name << "'\n" << infoLog << std::endl;
	};

	// Create and link the shader program with the vertex and fragment shaders.
	GLuint programId{ glCreateProgram() };
	glAttachShader(programId, vertexId);
	glAttachShader(programId, fragmentId);
	glLinkProgram(programId);

	// Print any linking errors.
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, 512, NULL, infoLog);
		std::cout << "ShaderLoader::loadFromStream: Program failed to link for '" <<
			name << "'\n" << infoLog << std::endl;
	}

	// Delete the shader objects after linking.
	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	// Create the shader asset and return it.
	if (success)
	{
		std::shared_ptr<Shader> shader{
			std::make_shared<Shader>(programId) };
		if (shader != nullptr)
		{
			std::cout << "ShaderLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
			return shader;
		}
	}
	
	return nullptr;
}