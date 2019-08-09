#pragma once
#ifndef Font_H
#define Font_H

#include "Texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

// The data for each generated character glyph.
struct Glyph
{
	glm::ivec2 size;
	glm::ivec2 bearing; // offset from baseline to top/left
	GLuint advance;		// offset between this and the next glyph
};

class Font : public Texture
{
public:
	Font(GLuint id, GLint width, GLint height,
		const std::vector<Glyph> &glyphs, const std::vector<glm::vec4> &clips);

	// Get the character glyph metrics.
	const std::vector<Glyph> &getGlyphs() const;

	// Get the texture clips.
	const std::vector<glm::vec4> &getClips() const;

private:
	// Holds the glyph's metrics.
	std::vector<Glyph> m_glyphs;

	// Holds the texture clips for the font's glyphs.
	std::vector<glm::vec4> m_clips;
};

#endif