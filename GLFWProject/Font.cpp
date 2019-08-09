#include "Font.h"

Font::Font(GLuint id, GLint width, GLint height,
	const std::vector<Glyph> &glyphs, const std::vector<glm::vec4> &clips) :
	Texture(id, width, height, 1), m_glyphs(glyphs), m_clips(clips)
{

}

const std::vector<Glyph> &Font::getGlyphs() const
{
	return m_glyphs;
}

const std::vector<glm::vec4> &Font::getClips() const
{
	return m_clips;
}