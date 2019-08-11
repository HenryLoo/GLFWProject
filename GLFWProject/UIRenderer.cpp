#include "UIRenderer.h"

#include "Camera.h"
#include "EntityConstants.h"
#include "AssetLoader.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace
{
	const GLfloat QUAD_VERTICES[] = {
		 -0.5f, -0.5f,
		  0.5f, -0.5f,
		 -0.5f,  0.5f,
		  0.5f,  0.5f,
	};

	const glm::vec2 HUD_POSITION{ 16.f, 16.f };
	const float HUD_SCALE{ 1.f };
}

UIRenderer::UIRenderer(AssetLoader *assetLoader)
{
	// Load resources.
	m_boxShader = assetLoader->load<Shader>("box");
	m_hudShader = assetLoader->load<Shader>("hud");
	m_hudFrame = assetLoader->load<Texture>("ui_frame");

	initBox();
	initHud();
}

UIRenderer::~UIRenderer()
{
	glDeleteBuffers(1, &m_boxVerticesVBO);
	glDeleteBuffers(1, &m_boxColourVBO);
	glDeleteBuffers(1, &m_boxModelViewsVBO);
	glDeleteVertexArrays(1, &m_boxVAO);

	glDeleteBuffers(1, &m_hudVerticesVBO);
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_boxColourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_boxColourVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance model view matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_hudModelsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(4, 1);
}

void UIRenderer::addBox(const GameComponent::Physics &physics,
	const AABB &aabb,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	// Push colour data.
	m_boxColourData.push_back(r);
	m_boxColourData.push_back(g);
	m_boxColourData.push_back(b);
	m_boxColourData.push_back(a);

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
	m_boxColourData.clear();
	m_boxModelViewsData.clear();
}

void UIRenderer::updateHud(glm::ivec2 windowSize)
{
	// Construct model view matrix for this sprite.
	glm::mat4 modelMatrix{ glm::mat4(1.0f) };

	// Apply translation.
	glm::vec2 hudSize{ glm::vec2(m_hudFrame->getSize()) * HUD_SCALE };
	glm::vec3 translation{ (-windowSize.x + hudSize.x) / 2.f + HUD_POSITION.x,
		(windowSize.y - hudSize.y) / 2.f - HUD_POSITION.x, 0.f };
	modelMatrix = glm::translate(modelMatrix, translation);

	// Apply scaling.
	glm::vec3 scale{ hudSize.x, hudSize.y, 1.f };
	modelMatrix = glm::scale(modelMatrix, scale);

	// Left-multiply by view matrix to get model view matrix.
	m_hudModelsData.push_back(modelMatrix);

	// Set the HUD data.
	glBindBuffer(GL_ARRAY_BUFFER, m_hudModelsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &m_hudModelsData[0], GL_STREAM_DRAW);
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

	m_hudFrame->bind();
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<int>(m_hudModelsData.size()));
}

void UIRenderer::renderBoxes(Camera *camera, glm::ivec2 windowSize)
{
	// Render the entity sprites.
	glBindVertexArray(m_boxVAO);

	// Update the instance buffers.
	int numBoxes{ static_cast<int>(m_boxModelViewsData.size()) };
	glBindBuffer(GL_ARRAY_BUFFER, m_boxColourVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numBoxes * sizeof(GLubyte) * 4, &m_boxColourData[0]);

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