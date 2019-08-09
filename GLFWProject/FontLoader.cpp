#include "FontLoader.h"

#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

std::shared_ptr<IAssetType> FontLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	if (flag <= 0)
	{
		std::cout << "FontLoader::loadFromStream: Font size must be larger than 0, given: " << flag << std::endl;
		return nullptr;
	}

	// Load font data from the buffer.
	int length{ streams[0].length };
	char *buffer{ new char[length] };
	streams[0].stream->read(buffer, length);

	// Initialize FreeType library.
	FT_Library library;
	if (FT_Init_FreeType(&library))
	{
		std::cout << "FontLoader::loadFromStream: Failed to initialize FreeType library" << std::endl;
		return nullptr;
	}

	// Load font from memory.
	FT_Face face;
	if (FT_New_Memory_Face(library, (unsigned char *)buffer, length, 0, &face))
	{
		std::cout << "FontLoader::loadFromStream: Failed to load font from memory" << std::endl;
		return nullptr;
	}

	// Set face size.
	// The flag is the font size.
	if (FT_Set_Pixel_Sizes(face, 0, flag))
	{
		std::cout << "FontLoader::loadFromStream: Failed to set font size" << std::endl;
		return nullptr;
	}

	// Disable byte-alignment restriction.
	// OpenGL requires that textures have size multiple of 4 bytes, but
	// since we are going to use only 1 byte per pixel (GL_RED), we need
	// to disable this alignment restriction.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set font metrics.
	GLuint cellW = 0;
	GLuint cellH = 0;
	int maxBearing = 0;
	int minHang = 0;
	const int NUM_CHARS{ 16 };
	const int TOTAL_CHARS{ NUM_CHARS * NUM_CHARS };
	FT_Glyph_Metrics metrics[TOTAL_CHARS];

	std::vector<GLubyte *> charPixels;
	std::vector<Glyph> glyphs;
	for (int i = 0; i < TOTAL_CHARS; ++i)
	{
		// Load the char glyph.
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			std::cout << "FontLoader::loadFromStream: Failed to load character glyph: << " << i << std::endl;
			continue;
		}

		// Get the metrics.
		metrics[i] = face->glyph->metrics;

		// Calculate max bearing.
		if (metrics[i].horiBearingY / 64 > maxBearing)
		{
			maxBearing = metrics[i].horiBearingY / 64;
		}

		// Calculate max width.
		if (static_cast<GLuint>(metrics[i].width / 64) > cellW)
		{
			cellW = metrics[i].width / 64;
		}

		// Calculate glyph hang.
		int glyphHang = (metrics[i].horiBearingY - metrics[i].height) / 64;
		if (glyphHang < minHang)
		{
			minHang = glyphHang;
		}

		// Store the character.
		GLuint width{ face->glyph->bitmap.width };
		GLuint height{ face->glyph->bitmap.rows };
		Glyph character = {
			glm::ivec2(width, height),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
		glyphs.push_back(character);

		// Store the character's pixels.
		GLuint size{ width * height };
		GLubyte *pixels = new GLubyte[size];
		memcpy(pixels, face->glyph->bitmap.buffer, size);
		charPixels.push_back(pixels);
	}

	// Create bitmap font.
	cellH = maxBearing - minHang;
	GLuint textureW{ cellW * NUM_CHARS };
	GLuint textureH{ cellH * NUM_CHARS };
	GLuint size{ textureW * textureH };
	GLubyte *texturePixels{ new GLubyte[size] };
	memset(texturePixels, 0, size);

	// Begin creating bitmap font.
	GLuint currentChar = 0;

	// Texture clip format: position = ( x, y ), width = z, height = w.
	glm::vec4 nextClip{ 0.f, 0.f, static_cast<float>(cellW),
		static_cast<float>(cellH) };
	std::vector<glm::vec4> clips;

	// Blitting coordinates.
	int bX = 0;
	int bY = 0;

	// Go through cell rows.
	for (int rows = 0; rows < NUM_CHARS; rows++)
	{
		// Go through each cell column in the row.
		for (int cols = 0; cols < NUM_CHARS; cols++)
		{
			// Set base offsets.
			bX = cellW * cols;
			bY = cellH * rows;

			// Initialize clip.
			nextClip.x = static_cast<float>(bX);
			nextClip.y = static_cast<float>(bY);
			nextClip.z = static_cast<float>(metrics[currentChar].width / 64);
			nextClip.w = static_cast<float>(cellH);

			// Blit character.
			Glyph ch{ glyphs[currentChar] };
			GLint x{ bX };
			GLint y{ bY + maxBearing - metrics[currentChar].horiBearingY / 64 };
			for (int i = 0; i < ch.size.y; ++i)
			{
				memcpy(&texturePixels[(i + y) * textureW + x],
					&charPixels[currentChar][i * ch.size.x], ch.size.x);
			}

			// Go to the next character.
			clips.push_back(nextClip);
			currentChar++;
		}
	}

	// Generate font texture.
	// Use GL_RED as the texture's internalFormat and format arguments,
	// since the generated bitmap is a grayscale 8-bit image.
	// Each colour is represented by 1 byte.
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		textureW,
		textureH,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		texturePixels
	);

	// Set texture options.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D, 0);

	// Release pixels.
	delete[] texturePixels;
	for (const GLubyte *pixels : charPixels)
	{
		delete[] pixels;
	}

	// Instantiate the font.
	std::shared_ptr<Font> font{ std::make_shared<Font>(textureId, textureW, 
		textureH, glyphs, clips) };
	if (font != nullptr)
	{
		std::cout << "FontLoader::loadFromStream: Loaded '" << name << "', font size: " << flag << "\n" << std::endl;
	}

	// Clean up.
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	// Delete the file buffer.
	delete[] buffer;

	return font;
}