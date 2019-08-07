#include "TextureLoader.h"

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::shared_ptr<IAssetType> TextureLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name)
{
	// If successfully created texture, instantiate the asset and return it.
	GLuint textureId;
	GLint width, height, numChannels;
	const IDataStream::Result &theResult{ streams[0] };
	if (loadValues(theResult, textureId, width, height, numChannels))
	{
		std::shared_ptr<Texture> texture{ 
			std::make_shared<Texture>(textureId, width, height, numChannels) };
		if (texture != nullptr)
		{
			std::cout << "TextureLoader::load: Loaded '" << name << "'\n" << std::endl;
			return texture;
		}
	}

	return nullptr;
}

bool TextureLoader::loadValues(const IDataStream::Result &streamedData,
	GLuint &textureId, GLint &width, GLint &height, GLint &numChannels)
{
	// OpenGL expects 0.0 of the y-axis to be on the bottom, but images have it
	// at the top. So we need to flip the image.
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the buffer.
	int length{ streamedData.length };
	char *buffer{ new char[length] };
	streamedData.stream->read(buffer, length);
	stbi_uc *data{ stbi_load_from_memory((stbi_uc *)buffer, length, &width, &height, &numChannels, 0) };

	bool success{ false };
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
		std::cout << "TextureLoader::loadValues: Failed to load texture." << std::endl;
	}

	// Free the image memory after we're done with it.
	stbi_image_free(data);
	delete buffer;

	return success;
}