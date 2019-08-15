#include "UIRenderer.h"

#include "Camera.h"
#include "EntityConstants.h"
#include "AssetLoader.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "TextRenderer.h"
#include "Font.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

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
	const std::string HUD_FRAME{ "frame" };

	const glm::vec2 HEALTH_BAR_OFFSET{ 74.f, 3.f };
	const glm::vec2 RESOURCE_BAR_OFFSET{ HEALTH_BAR_OFFSET.x, 35.f };
	const float MAX_BAR_WIDTH{ 259.f };
	const std::string HUD_HEALTH_BAR{ "health_bar" };
	const std::string HUD_RESOURCE_BAR{ "resource_bar" };

	const glm::vec2 SKILL_ICON_OFFSET{ 52.f, 78.f };
	const glm::vec2 SKILL_ICON_INNER_OFFSET{ 3.f, 3.f };
	const std::string HUD_SKILL_ICON{ "skill_icon" };

	const glm::vec2 PORTRAIT_ICON_OFFSET{ 5.f, 5.f };
}

UIRenderer::UIRenderer(AssetLoader *assetLoader)
{
	// Load resources.
	m_boxShader = assetLoader->load<Shader>("box");
	m_hudShader = assetLoader->load<Shader>("sprite");
	m_hudTexture = assetLoader->load<SpriteSheet>("hud");
	m_hudFont = assetLoader->load<Font>("default", 16);

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
	glVertexAttribDivisorARB(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_boxColoursVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glVertexAttribDivisorARB(1, 1);

	// Create the VBO for instance model view matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_boxModelViewsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxModelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribDivisorARB(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribDivisorARB(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(5, 1);
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
	glVertexAttribDivisorARB(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudColoursVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glVertexAttribDivisorARB(1, 1);

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
	glVertexAttribDivisorARB(2, 1);

	// Create the VBO for instance model matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudModelsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribDivisorARB(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribDivisorARB(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(6, 1);
}

void UIRenderer::addBox(const GameComponent::Physics &physics,
	const AABB &aabb, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
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

	m_hudColoursData.clear();
	m_hudTexCoordsData.clear();
	m_hudModelsData.clear();
}

void UIRenderer::addHudElement(std::string elementType,
	glm::vec2 offset, glm::vec2 scale,
	GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	// Check if the hud element exists.
	SpriteSheet::SpriteSet thisSprite;
	bool hasHud{ m_hudTexture->getSprite(elementType, thisSprite) };
	if (!hasHud)
		return;

	// Push colour data.
	m_hudColoursData.push_back(r);
	m_hudColoursData.push_back(g);
	m_hudColoursData.push_back(b);
	m_hudColoursData.push_back(a);

	// Construct model matrix for this sprite.
	glm::mat4 hudModel{ glm::mat4(1.0f) };

	// Apply translation.
	// Only one sprite per set, so just take the first clip.
	const SpriteSheet::SpriteClip &thisClip{ thisSprite.clips[0] };
	glm::vec2 scaledHudSize{ glm::vec2(thisClip.clipSize) * HUD_SCALE };
	glm::vec3 hudTrans{ 
		(-m_windowSize.x + scaledHudSize.x + scale.x) / 2.f + HUD_POSITION.x + offset.x,
		(m_windowSize.y - scaledHudSize.y) / 2.f - HUD_POSITION.x - offset.y,
		0.f 
	};
	hudModel = glm::translate(hudModel, hudTrans);

	// Apply scaling.
	glm::vec3 hudScale{ scaledHudSize.x * scale.x, scaledHudSize.y, 1.f };
	hudModel = glm::scale(hudModel, hudScale);

	// Push model data.
	m_hudModelsData.push_back(hudModel);

	// Push texture coordinate data.
	glm::vec2 hudSize{ m_hudTexture->getSize() };
	m_hudTexCoordsData.push_back(thisClip.topLeft.x / hudSize.x);
	m_hudTexCoordsData.push_back(1.f - thisClip.topLeft.y / hudSize.y);
	m_hudTexCoordsData.push_back(thisClip.clipSize.x / hudSize.x);
	m_hudTexCoordsData.push_back(thisClip.clipSize.y / hudSize.y);
}

void UIRenderer::updateHud(AssetLoader *assetLoader, TextRenderer *tRenderer,
	const std::string &portraitIcon,
	const std::vector<std::string> &skillIcons,
	int currentHealth, int maxHealth, int currentResource, int maxResource,
	const std::vector<float> &cooldowns)
{
	// Add frame.
	addHudElement(HUD_FRAME);

	// Add portrait icon.
	addHudElement(portraitIcon, PORTRAIT_ICON_OFFSET);

	// Add health bar.
	const float CURRENT_HEALTH{ static_cast<float>(currentHealth) };
	const float MAX_HEALTH{ static_cast<float>(glm::max(1, maxHealth)) };
	float healthAmount{ CURRENT_HEALTH / MAX_HEALTH * MAX_BAR_WIDTH };
	addHudElement(HUD_HEALTH_BAR, HEALTH_BAR_OFFSET, glm::vec2(healthAmount, 1.f));

	// Add resource bar.
	const float CURRENT_RESOURCE{ static_cast<float>(currentResource) };
	const float MAX_RESOURCE{ static_cast<float>(glm::max(1, maxResource)) };
	float resourceAmount{ CURRENT_RESOURCE / MAX_RESOURCE * MAX_BAR_WIDTH };
	addHudElement(HUD_RESOURCE_BAR, RESOURCE_BAR_OFFSET, glm::vec2(resourceAmount, 1.f));

	// Add skill icons.
	for (int i = 0; i < cooldowns.size(); ++i)
	{
		glm::vec2 offset{ SKILL_ICON_OFFSET };
		offset.x *= i;
		addHudElement(HUD_SKILL_ICON, offset);
	}

	for (int i = 0; i < skillIcons.size(); ++i)
	{
		glm::vec2 offset{ SKILL_ICON_OFFSET };
		offset.x *= i;
		offset += SKILL_ICON_INNER_OFFSET;

		// Lower opacity if the skill is on cooldown.
		GLubyte alpha{ 255 };
		if (cooldowns[i] > 0.f)
		{
			alpha = 100;
		}
		const std::string &icon{ skillIcons[i] };
		addHudElement(icon, offset, glm::vec2(1.f), 255, 255, 255, alpha);

		++i;
	}

	// Add health and resource text.
	SpriteSheet::SpriteSet barSprite;
	m_hudTexture->getSprite(HUD_HEALTH_BAR, barSprite);
	const int BAR_HEIGHT{ barSprite.clips[0].clipSize.y };

	std::string healthStr{ std::to_string(currentHealth) + " / " +
		std::to_string(maxHealth) };
	glm::vec2 healthPos{ HUD_POSITION + HEALTH_BAR_OFFSET };
	tRenderer->addText(healthStr, m_hudFont.get(), 
		{ healthPos, { MAX_BAR_WIDTH, BAR_HEIGHT } }, TextAlign::CENTER, true);

	std::string resourceStr{ std::to_string(currentResource) + " / " +
		std::to_string(maxResource) };
	glm::vec2 resourcePos{ HUD_POSITION + RESOURCE_BAR_OFFSET };
	tRenderer->addText(resourceStr, m_hudFont.get(), 
		{ resourcePos, { MAX_BAR_WIDTH, BAR_HEIGHT } }, TextAlign::CENTER, true);

	// Add skill cooldown text.
	SpriteSheet::SpriteSet iconSprite;
	m_hudTexture->getSprite(HUD_SKILL_ICON, iconSprite);
	const glm::vec2 ICON_SIZE{ iconSprite.clips[0].clipSize };
	for (int i = 0; i < cooldowns.size(); ++i)
	{
		float cd{ cooldowns[i] };

		// Only show text if skill is on cooldown.
		if (cd == 0.f)
			continue;

		// Show up to 1 decimal place.
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << cd;
		std::string cdStr{ ss.str() + "s" };

		glm::vec2 cdPos{ SKILL_ICON_OFFSET };
		cdPos.x *= i;
		cdPos += HUD_POSITION;
		tRenderer->addText(cdStr, m_hudFont.get(),
			{ cdPos, ICON_SIZE }, TextAlign::CENTER, true);
	}
}

void UIRenderer::renderHud()
{
	// Render HUD elements.
	glBindVertexArray(m_hudVAO);

	// Use the shader.
	m_hudShader->use();

	// Bind to texture unit 4, since 1-3 are being used by SpriteRenderer.
	glActiveTexture(GL_TEXTURE4);
	m_hudShader->setInt("textureSampler", 4);
	m_hudShader->setVec2("textureSize", m_hudTexture->getSize());

	// Orthographic projection with origin of the coordinate space defined at
	// the center of the screen. Negative y-axis points down.
	glm::mat4 projectionMatrix{ getProjectionMatrix(1.f) };
	m_hudShader->setMat4("projection", projectionMatrix);

	// Set the HUD data.
	int numElements{ static_cast<int>(m_hudModelsData.size()) };
	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(GLubyte) * 4, NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(GLfloat) * 4, NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, numElements * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudColoursVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(GLubyte) * 4, &m_hudColoursData[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudTexCoordsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(GLfloat) * 4, &m_hudTexCoordsData[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numElements * sizeof(glm::mat4), &m_hudModelsData[0]);

	m_hudTexture->bind();
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numElements);
}

void UIRenderer::renderBoxes(Camera *camera)
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