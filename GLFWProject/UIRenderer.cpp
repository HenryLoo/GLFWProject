#include "UIRenderer.h"

#include "Camera.h"
#include "EntityConstants.h"
#include "AssetLoader.h"
#include "Shader.h"
#include "SpriteSheet.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace
{
	const GLfloat QUAD_VERTICES[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};

	const glm::vec2 HUD_POSITION{ 16.f, 16.f };
	const float HUD_SCALE{ 1.f };
	const glm::vec2 HEALTH_BAR_OFFSET{ 74.f, 3.f };
	const glm::vec2 RESOURCE_BAR_OFFSET{ HEALTH_BAR_OFFSET.x, 35.f };
	const float MAX_BAR_WIDTH{ 259.f };
	const std::string HUD_FRAME{ "frame" };
	const std::string HUD_HEALTHBAR{ "health_bar" };
	const std::string HUD_RESOURCEBAR{ "resource_bar" };
	const std::string HUD_SKILLICON{ "skill_icon" };
}

UIRenderer::UIRenderer(AssetLoader *assetLoader)
{
	// Load resources.
	m_boxShader = assetLoader->load<Shader>("box");
	m_hudShader = assetLoader->load<Shader>("sprite");
	m_hudTexture = assetLoader->load<SpriteSheet>("hud");

	initBox();
	initHud();
}

UIRenderer::~UIRenderer()
{
	glDeleteBuffers(1, &m_boxVerticesVBO);
	glDeleteBuffers(1, &m_boxColoursVBO);
	glDeleteBuffers(1, &m_boxModelViewsVBO);
	glDeleteVertexArrays(1, &m_boxVAO);

	glDeleteBuffers(1, &m_hudVerticesVBO);
	glDeleteBuffers(1, &m_hudColoursVBO);
	glDeleteBuffers(1, &m_hudTexCoordsVBO);
	glDeleteBuffers(1, &m_hudModelsVBO);
	glDeleteVertexArrays(1, &m_hudVAO);
}

void UIRenderer::initBox()
{

	// Create the vertex array object and bind to it.
	// All subsequent VBO configurations will be saved for this VAO.
	glGenVertexArrays(1, &m_boxVAO);
	glBindVertexArray(m_boxVAO);

	// Define the vertex data for a sprite quad, for the VBO.
	// This is shared by all instances.
	glGenBuffers(1, &m_boxVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

	// Set attribute for vertices.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_boxColoursVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance model view matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_boxModelViewsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxModelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(5, 1);
}

void UIRenderer::initHud()
{
	// Create the vertex array object and bind to it.
	// All subsequent VBO configurations will be saved for this VAO.
	glGenVertexArrays(1, &m_hudVAO);
	glBindVertexArray(m_hudVAO);

	// Define the vertex data for a sprite quad, for the VBO.
	// This is shared by all instances.
	glGenBuffers(1, &m_hudVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

	// Set attribute for vertices.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudColoursVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance texture coordinates.
	// Each vertex holds 4 values: u, v of the top-left texture point, 
	// and the normalized width, height of the clip size.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudTexCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// Set attribute for instance texture coordinates.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(2, 1);

	// Create the VBO for instance model matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudModelsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(6, 1);
}

void UIRenderer::addBox(const GameComponent::Physics &physics,
	const AABB &aabb,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	// Push colour data.
	m_boxColoursData.push_back(r);
	m_boxColoursData.push_back(g);
	m_boxColoursData.push_back(b);
	m_boxColoursData.push_back(a);

	// Construct model view matrix for this sprite.
	glm::mat4 modelMatrix{ glm::mat4(1.0f) };

	// Apply translation.
	glm::vec3 translation{
		physics.pos.x + physics.scale.x * aabb.offset.x,
		physics.pos.y + physics.scale.y * aabb.offset.y,
		physics.pos.z
	};
	modelMatrix = glm::translate(modelMatrix, translation);

	// Apply scaling.
	glm::vec3 scale{
		aabb.halfSize.x * 2,
		aabb.halfSize.y * 2,
		1.f
	};
	modelMatrix = glm::scale(modelMatrix, scale);

	// Left-multiply by view matrix to get model view matrix.
	glm::mat4 modelViewMatrix{ m_viewMatrix * modelMatrix };

	m_boxModelViewsData.push_back(modelViewMatrix);
}

void UIRenderer::resetData()
{
	// Clear data.
	m_boxColoursData.clear();
	m_boxModelViewsData.clear();
}

void UIRenderer::renderHud(glm::ivec2 windowSize)
{
	// Render HUD elements.
	glBindVertexArray(m_hudVAO);

	// Use the shader.
	m_hudShader->use();

	// Bind to texture unit 4, since 1-3 are being used by SpriteRenderer.
	glActiveTexture(GL_TEXTURE4);
	m_hudShader->setInt("textureSampler", 4);

	// Orthographic projection with origin of the coordinate space defined at
	// the center of the screen. Negative y-axis points down.
	glm::mat4 projectionMatrix{ getProjectionMatrix(1.f) };
	m_hudShader->setMat4("projection", projectionMatrix);

	// Draw the frame.
	const std::vector<GLubyte> HUD_COLOURS{ 255, 255, 255, 255,  255, 255, 255,
		255, 255, 255, 255, 255 };

	// Construct model matrix for this sprite.
	glm::mat4 hudModel{ glm::mat4(1.0f) };

	// Apply translation.
	SpriteSheet::SpriteSet thisSprite;
	bool hasHud{ m_hudTexture->getSprite(HUD_FRAME, thisSprite) };
	const SpriteSheet::SpriteClip &thisClip{ thisSprite.clips[0] };
	glm::vec2 scaledHudSize{ glm::vec2(thisClip.clipSize) * HUD_SCALE };
	glm::vec3 hudTrans{ (-windowSize.x + scaledHudSize.x) / 2.f + HUD_POSITION.x,
		(windowSize.y - scaledHudSize.y) / 2.f - HUD_POSITION.x, 0.f };
	hudModel = glm::translate(hudModel, hudTrans);

	// Apply scaling.
	glm::vec3 hudScale{ scaledHudSize.x, scaledHudSize.y, 1.f };
	hudModel = glm::scale(hudModel, hudScale);

	std::vector<glm::mat4> hudModels{ hudModel };

	glm::vec2 hudSize{ m_hudTexture->getSize() };
	std::vector<GLfloat> hudTexCoords{ 
		thisClip.topLeft.x / hudSize.x,
		1.f - thisClip.topLeft.y / hudSize.y,
		thisClip.clipSize.x / hudSize.x,
		thisClip.clipSize.y / hudSize.y
	};

	// Draw the health bar.
	hudModel = glm::mat4(1.0f);

	// Apply translation.
	hasHud = m_hudTexture->getSprite(HUD_HEALTHBAR, thisSprite);
	const SpriteSheet::SpriteClip &healthClip{ thisSprite.clips[0] };
	scaledHudSize = glm::vec2(healthClip.clipSize) * HUD_SCALE;
	const float CURRENT_HEALTH{ 100.f };
	const float MAX_HEALTH{ 100.f };
	float healthAmount{ CURRENT_HEALTH / MAX_HEALTH * MAX_BAR_WIDTH };
	hudTrans = {
		(-windowSize.x + scaledHudSize.x + healthAmount) / 2.f + HUD_POSITION.x + HEALTH_BAR_OFFSET.x,
		(windowSize.y - scaledHudSize.y) / 2.f - HUD_POSITION.x - HEALTH_BAR_OFFSET.y, 0.f
	};
	hudModel = glm::translate(hudModel, hudTrans);

	// Apply scaling.
	hudScale = {scaledHudSize.x * healthAmount, scaledHudSize.y, 1.f };
	hudModel = glm::scale(hudModel, hudScale);

	hudModels.push_back(hudModel);
	hudTexCoords.push_back(healthClip.topLeft.x / hudSize.x);
	hudTexCoords.push_back(1.f - healthClip.topLeft.y / hudSize.y);
	hudTexCoords.push_back(healthClip.clipSize.x / hudSize.x);
	hudTexCoords.push_back(healthClip.clipSize.y / hudSize.y);

	// Draw the resource bar.

	// Apply translation.
	hudModel = glm::mat4(1.0f);

	hasHud = m_hudTexture->getSprite(HUD_RESOURCEBAR, thisSprite);
	const SpriteSheet::SpriteClip &resourceClip{ thisSprite.clips[0] };
	scaledHudSize = glm::vec2(resourceClip.clipSize) * HUD_SCALE;
	const float CURRENT_RESOURCE{ 100.f };
	const float MAX_RESOURCE{ 100.f };
	float resourceAmount{ CURRENT_RESOURCE / MAX_RESOURCE * MAX_BAR_WIDTH };
	hudTrans = {
		(-windowSize.x + scaledHudSize.x + resourceAmount) / 2.f + HUD_POSITION.x + RESOURCE_BAR_OFFSET.x,
		(windowSize.y - scaledHudSize.y) / 2.f - HUD_POSITION.x - RESOURCE_BAR_OFFSET.y, 0.f
	};
	hudModel = glm::translate(hudModel, hudTrans);

	// Apply scaling.
	hudScale = { scaledHudSize.x * resourceAmount, scaledHudSize.y, 1.f };
	hudModel = glm::scale(hudModel, hudScale);

	hudModels.push_back(hudModel);
	hudTexCoords.push_back(resourceClip.topLeft.x / hudSize.x);
	hudTexCoords.push_back(1.f - resourceClip.topLeft.y / hudSize.y);
	hudTexCoords.push_back(resourceClip.clipSize.x / hudSize.x);
	hudTexCoords.push_back(resourceClip.clipSize.y / hudSize.y);

	// Set the HUD data.
	int numElements{ static_cast<int>(hudModels.size()) };
	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(GLubyte) * 4, NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(GLfloat) * 4, NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(GLubyte) * 4, &HUD_COLOURS[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(GLfloat) * 4, &hudTexCoords[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(glm::mat4), &hudModels[0]);

	m_hudTexture->bind();
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numElements);

	

	//int numBars{ static_cast<int>(barModels.size()) };
	//glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, numBars * sizeof(GLubyte) * 4, &barColours[0]);

	//glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, numBars * sizeof(GLfloat) * 4, &barTexCoords[0]);

	//glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, numBars * sizeof(glm::mat4), &barModels[0]);

	//m_hudBar->bind();
	//glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numBars);
}

void UIRenderer::renderBoxes(Camera *camera, glm::ivec2 windowSize)
{
	// Render the entity sprites.
	glBindVertexArray(m_boxVAO);

	// Update the instance buffers.
	int numBoxes{ static_cast<int>(m_boxModelViewsData.size()) };
	glBindBuffer(GL_ARRAY_BUFFER, m_boxColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numBoxes * sizeof(GLubyte) * 4, &m_boxColoursData[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_boxModelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numBoxes * sizeof(glm::mat4), &m_boxModelViewsData[0]);

	// Use the shader.
	m_boxShader->use();

	// Set the camera uniforms.
	// Orthographic projection with origin of the coordinate space defined at
	// the center of the screen. Negative y-axis points down.
	glm::mat4 projectionMatrix{ getProjectionMatrix(camera->getZoom()) };
	m_boxShader->setMat4("projection", projectionMatrix);

	// Draw the instances.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numBoxes);
}