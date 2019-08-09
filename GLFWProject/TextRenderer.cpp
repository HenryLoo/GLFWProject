#include "TextRenderer.h"

#include "AssetLoader.h"
#include "Shader.h"
#include "Font.h"

#include <glm/gtc/matrix_transform.hpp>

TextRenderer::TextRenderer(AssetLoader *assetLoader)
{
	// Load the text shader.
	m_shader = assetLoader->load<Shader>("text");

	if (m_shader != nullptr)
	{
		// Configure shaders.
		m_shader->use();
		m_shader->setInt("text", 0);
	}

	// Configure vertex array object.
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Configure vertex buffer object.
	GLfloat vertices[]
	{
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
		GL_STATIC_DRAW);

	// Configure element array buffer object.
	GLuint elements[]
	{
		0, 1, 2,
		0, 3, 1
	};

	// Configure element buffer object.
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
		GL_STATIC_DRAW);

	// Configure vertex attributes.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		(GLvoid *)0);

	// Configure clip buffer object.
	glGenBuffers(1, &m_clipVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_clipVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		(GLvoid *)0);
	glVertexAttribDivisor(1, 1);

	// Configure colour buffer object.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		(GLvoid *)0);
	glVertexAttribDivisor(2, 1);

	// Configure model buffer object.
	glGenBuffers(1, &m_modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_modelVBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4),
		(GLvoid *)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4),
		(GLvoid *) sizeof(glm::vec4));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4),
		(GLvoid *)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4),
		(GLvoid *)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	// Unbind VBO and VAO.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextRenderer::~TextRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void TextRenderer::addText(const std::string &text, Font *font,
	glm::vec2 pos, glm::vec4 colour, GLfloat scale)
{
	addText(text, font, { pos.x, pos.y, 0.f, 0.f }, colour,
	TextAlign::LEFT, false, scale);
}

void TextRenderer::addText(const std::string &text, Font *font,
	const TextBox &quad, glm::vec4 colour, TextAlign align,
	bool isVerticalCenter, GLfloat scale)
{
	if (font == nullptr)
		return;

	auto it{ m_texts.find(font) };

	// This font is already being used by some other text.
	if (it != m_texts.end())
	{
		m_texts[font].push_back({ text, quad, colour, align,
			isVerticalCenter, scale });
	}
	// No text already using this font.
	else
	{
		m_texts.insert({ font, std::vector<TextProperties>{
			{ text, quad, colour, align, isVerticalCenter, scale }
		} });
	}
}

void TextRenderer::render(glm::vec2 screenSize)
{
	if (m_texts.empty())
		return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set viewport.
	glViewport(0, 0, static_cast<GLsizei>(screenSize.x), 
		static_cast<GLsizei>(screenSize.y));

	// Configure shader.
	m_shader->use();

	// Iterate through in-use fonts.
	for (auto it = m_texts.begin(); it != m_texts.end(); ++it)
	{
		Font *thisFont{ it->first };
		const std::vector<TextProperties> &thisTexts{ it->second };

		// Instance vectors.
		std::vector<glm::vec4> clips;
		std::vector<glm::vec4> colours;
		std::vector<glm::mat4> modelMatrices;
		
		// Get font attributes.
		const std::vector<Glyph> &chars{ thisFont->getGlyphs() };
		const std::vector<glm::vec4> &fontClips{ thisFont->getClips() };
		const glm::ivec2 textureSize{ thisFont->getSize() };

		// Iterate through all text strings using this font.
		for (const TextProperties &thisText : thisTexts)
		{
			// Iterate through each char in the text string to get its
			// bounding width and height.
			std::string::const_iterator c;
			float alignOffsetX{ 0.f };
			if (thisText.align != TextAlign::LEFT)
			{
				float textWidth{ 0.f };
				for (c = thisText.text.begin(); c != thisText.text.end(); ++c)
				{
					// The current character.
					Glyph character{ chars[*c] };

					// Add to total text width.
					textWidth += (character.advance >> 6) * thisText.scale;
				}

				// Calculate alignment offsets.
				if (thisText.align == TextAlign::CENTER)
				{
					alignOffsetX = (thisText.quad.w - textWidth) / 2;
				}
				else if (thisText.align == TextAlign::RIGHT)
				{
					alignOffsetX = thisText.quad.w - textWidth;
				}
			}

			// Iterate through each char in the text string to construct
			// its instance data.
			int index{ 0 };
			float x{ thisText.quad.x + alignOffsetX };
			for (c = thisText.text.begin(); c != thisText.text.end(); ++c)
			{
				// The current character.
				Glyph character{ chars[*c] };

				// Set the character's clip.
				glm::vec4 clip{ fontClips[*c] };
				clips.push_back(glm::vec4(
					static_cast<float>(clip.x) / textureSize.x,
					static_cast<float>(clip.y) / textureSize.y,
					static_cast<float>(clip.z) / textureSize.x,
					static_cast<float>(clip.w) / textureSize.y
				));
				
				// Set the character's colour.
				colours.push_back(thisText.colour);

				// Set positions.
				// Glyphs are already vertically aligned when constructing the texture 
				// atlas, so bearing.y is unnecessary.
				GLfloat posX{ x + character.bearing.x * thisText.scale };
				GLfloat posY{ thisText.quad.y * thisText.scale };

				if (thisText.isVerticalCenter)
				{
					posY += ((thisText.quad.h - clip.w) / 2.f * thisText.scale);
					posY = roundf(posY); // removes some artifacts
				}

				// Set sizes.
				GLfloat w{ clip.z * thisText.scale };
				GLfloat h{ clip.w * thisText.scale };

				// Prepare the model matrix for this character.
				glm::mat4 modelMatrix{ glm::mat4(1.0f) };
				modelMatrix = glm::translate(modelMatrix, glm::vec3(posX, posY, 0.f));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(w, h, 1.f));
				modelMatrices.push_back(modelMatrix);

				// Move the cursor forward for the next glyph.
				// Bitshift by 6 to get the value in pixels, since ( 2^6 = 64 and the 
				// value is measured in 1/64th of a pixel ).
				x += (character.advance >> 6) * thisText.scale;

				++index;
			}
		}

		// Bind clips.
		glBindBuffer(GL_ARRAY_BUFFER, m_clipVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * clips.size(),
			&clips[0], GL_STATIC_DRAW);
		
		// Bind colours.
		glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * colours.size(),
			&colours[0], GL_STATIC_DRAW);

		// Bind model instances.
		glBindBuffer(GL_ARRAY_BUFFER, m_modelVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * modelMatrices.size(),
			&modelMatrices[0], GL_STATIC_DRAW);

		// Unbind VBO.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Bind to the font texture atlas.
		glActiveTexture(GL_TEXTURE0);
		thisFont->bind();

		// Draw text.
		glBindVertexArray(m_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(modelMatrices.size()));

		// Unbind VAO.
		glBindVertexArray(0);

		// Unbind texture.
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Clear text instances.
	m_texts.clear();
}

void TextRenderer::setProjectionMatrix(glm::vec2 screenSize)
{
	// Use orthographic projection, since we won't be needing perspective for text.
	// This allows us to use vertex coordinates as screen coordinates.
	m_shader->use();
	m_shader->setMat4("projection", glm::ortho(0.0f, screenSize.x, screenSize.y, 0.0f));
}