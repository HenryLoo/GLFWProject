#pragma once
#ifndef TextRenderer_H
#define TextRenderer_H

#include "AABB.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

struct TextBox
{
	float x;
	float y;
	float w;
	float h;
};

enum class TextAlign
{
	LEFT,
	CENTER,
	RIGHT
};

struct TextProperties
{
	std::string text;
	TextBox quad;
	glm::vec4 colour;
	TextAlign align;
	bool isVerticalCenter;
	GLfloat scale;
};

class GameEngine;
class AssetLoader;
class Font;
class Shader;

class TextRenderer
{
public:
	TextRenderer(AssetLoader *assetLoader);
	~TextRenderer();

	// Add a text string to be rendered.
	void addText(const std::string &text, Font *font, glm::vec2 pos,
		glm::vec4 colour = glm::vec4(1.f), GLfloat scale = 1.f);
	void addText(const std::string &text, Font *font,
		const TextBox &quad, glm::vec4 colour = glm::vec4(1.f),
		TextAlign align = TextAlign::LEFT, bool isVerticalCenter = false,
		GLfloat scale = 1.f);

	// Render the queued text strings in the map, where strings with the
	// same font are rendered together.
	// Call this after drawing all the text strings for this game loop.
	void render(glm::vec2 screenSize);

	// Set the projection matrix based on current screen size.
	void setProjectionMatrix(glm::vec2 screenSize);

private:
	// Hold the text shader.
	std::shared_ptr<Shader> m_shader;

	// The vertex array object.
	GLuint m_VAO;

	// Instance VBOs.
	GLuint m_clipVBO, m_colourVBO, m_modelVBO;

	// Group text strings to render by common fonts, to minimize texture binds
	// and draw calls.
	std::map<Font *, std::vector<TextProperties>> m_texts;
};

#endif