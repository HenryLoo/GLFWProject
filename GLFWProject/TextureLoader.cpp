#include "TextureLoader.h"

//#include "Texture.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

IAssetType *TextureLoader::load(std::iostream *stream, int length)
{
	bool success{ false };

	// OpenGL expects 0.0 of the y-axis to be on the bottom, but images have it
	// at the top. So we need to flip the image.
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file.
	//const std::string path{ TEXTURE_PATH + filePath };
	//stbi_uc *data{ stbi_load(path.c_str(), &m_width, &m_height, &m_numChannels, 0) };
	char *buffer{ new char[length] };
	stream->read(buffer, length);
	int width, height, numChannels;
	stbi_uc *data{ stbi_load_from_memory((stbi_uc *)buffer, length, &width, &height, &numChannels, 0) };

	// TODO: Demonstrating getting rgba values from pixels. 
	// Can use this to define room layouts from images. Remove later.
	int x = 0;
	int y = 2;
	const stbi_uc *p{ data + (4 * (y * width + x)) };
	stbi_uc r{ p[0] };
	stbi_uc g{ p[1] };
	stbi_uc b{ p[2] };
	stbi_uc a{ p[3] };

	GLuint textureId;
	if (data != nullptr)
	{
		// Generate the texture for OpenGL and store its id.
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Set texture parameters for the bound texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Create the texture from the loaded file.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);

		success = true;
	}
	else
	{
		std::cout << "Failed to load texture." << std::endl;
	}

	// Free the image memory after we're done with it.
	stbi_image_free(data);

	// If successfully created texture, instantiate the asset and return it.
	if (success)
	{
		/*Texture *texture(textureId, width, height, numChannels);
		return texture;*/
	}

	return nullptr;
}