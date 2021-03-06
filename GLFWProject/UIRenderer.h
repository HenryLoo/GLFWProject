#pragma once
#ifndef UIRenderer_H
#define UIRenderer_H

#include "Renderer.h"
#include "GameComponent.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

class Camera;
class Shader;
class AssetLoader;
class Texture;
class TextRenderer;
class Font;

class UIRenderer : public Renderer
{
public:
	UIRenderer(AssetLoader *assetLoader);
	~UIRenderer();

	// Initialize box vertex buffers.
	void initBox();

	// Initialize HUD elements.
	void initHud();

	// Add box data to the array of boxes to prepare for rendering.
	// The rgba colour values range from 0-255.
	void addBox(const GameComponent::Physics &physics, 
		const AABB &aabb, GLubyte r, GLubyte g, GLubyte b, GLubyte a);

	// Add data for a HUD element to prepare for rendering.
	// The rgba colour values range from 0-255.
	void addHudElement(std::string elementType,
		glm::vec2 offset = { 0.f, 0.f }, glm::vec2 scale = { 1.f, 1.f },
		GLubyte r = 255, GLubyte g = 255, GLubyte b = 255, GLubyte a = 255 );

	// Update the HUD with dynamic values.
	void updateHud(AssetLoader *assetLoader, TextRenderer *tRenderer, 
		const std::string &portraitIcon, 
		const std::vector<std::string> &skillIcons,
		int currentHealth, int maxHealth, int currentResource, int maxResource,
		const std::vector<float> &cooldowns);

	// Set the number of boxes to render to 0.
	// This should be called every frame from the update loop, so that
	// the proper number of boxes can be recalculated.
	virtual void resetData();

	// Render the ui and all queued boxes.
	void renderHud();
	void renderBoxes(Camera* camera);

private:
	// Shaders to render with.
	std::shared_ptr<Shader> m_boxShader;
	std::shared_ptr<Shader> m_hudShader;

	// Data to send to the GPU.
	std::vector<GLubyte> m_boxColoursData;
	std::vector<glm::mat4> m_boxModelsData;
	std::vector<GLubyte> m_hudColoursData;
	std::vector<float> m_hudTexCoordsData;
	std::vector<glm::mat4> m_hudModelsData;

	// The vertex array object and vertex buffer object for instances.
	GLuint m_boxVAO, m_boxVerticesVBO, m_boxColoursVBO, m_boxModelsVBO;
	GLuint m_hudVAO, m_hudVerticesVBO, m_hudColoursVBO, m_hudTexCoordsVBO, m_hudModelsVBO;

	// Hold HUD assets.
	std::shared_ptr<SpriteSheet> m_hudTexture;
	std::shared_ptr<Font> m_hudFont;
};

#endif