#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace
{
	const std::string TEXTURE_PATH = "textures/";
}

Texture::Texture(const std::string &filePath)
{
	// OpenGL expects 0.0 of the y-axis to be on the bottom, but images have it
	// at the top. So we need to flip the image.
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file.
	const std::string path{ TEXTURE_PATH + filePath };
	unsigned char *data{ stbi_load(path.c_str(), &m_width, &m_height, &m_numChannels, 0) };

	if (data != nullptr)
	{
		// Generate the texture for OpenGL and store its id.
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		// Set texture parameters for the bound texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Create the texture from the loaded file.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
	}
	else
	{
		std::cout << "Failed to load texture." << std::endl;
	}


	// Free the image memory after we're done with it.
	stbi_image_free(data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, m_id);
}

glm::vec2 Texture::getSize() const
{
	return glm::vec2(m_width, m_height);
}