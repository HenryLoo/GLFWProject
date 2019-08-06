#include "UIRenderer.h"

#include "Camera.h"
#include "EntityConstants.h"
#include "AssetLoader.h"
#include "Shader.h"

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
}

UIRenderer::UIRenderer(AssetLoader *assetLoader)
{
	// TODO: replace these hardcoded resources.
	m_boxShader = assetLoader->load<Shader>("box");

	// Prepare the data buffers.
	m_posSizeData = new GLfloat[EntityConstants::MAX_ENTITIES * 4];
	m_colourData = new GLubyte[EntityConstants::MAX_ENTITIES * 4];

	// Create the vertex array object and bind to it.
	// All subsequent VBO configurations will be saved for this VAO.
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Define the vertex data for a sprite quad, for the VBO.
	// This is shared by all instances.
	glGenBuffers(1, &m_verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

	// Set attribute for vertices.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance positions and sizes.
	// Each vertex holds 4 values: x, y, scaleX, scaleY.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_posSizeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_posSizeVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// Set attribute for instance positions and sizes.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)0);
	glVertexAttribDivisor(2, 1);
}

UIRenderer::~UIRenderer()
{
	delete[] m_posSizeData;
	delete[] m_colourData;

	glDeleteBuffers(1, &m_colourVBO);
	glDeleteBuffers(1, &m_posSizeVBO);
	glDeleteBuffers(1, &m_verticesVBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void UIRenderer::addBox(const GameComponent::Physics &physics,
	const AABB &aabb,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	m_posSizeData[4 * m_numBoxes] = physics.pos.x + physics.scale.x * aabb.offset.x;
	m_posSizeData[4 * m_numBoxes + 1] = physics.pos.y + physics.scale.y * aabb.offset.y;
	m_posSizeData[4 * m_numBoxes + 2] = aabb.halfSize.x * 2;
	m_posSizeData[4 * m_numBoxes + 3] = aabb.halfSize.y * 2;

	m_colourData[4 * m_numBoxes] = r;
	m_colourData[4 * m_numBoxes + 1] = g;
	m_colourData[4 * m_numBoxes + 2] = b;
	m_colourData[4 * m_numBoxes + 3] = a;

	m_numBoxes++;
}

void UIRenderer::resetNumBoxes()
{
	m_numBoxes = 0;
}

void UIRenderer::render(Camera *camera, glm::ivec2 windowSize)
{
	// Orthographic projection with origin of the coordinate space defined at
	// the center of the screen. Negative y-axis points down.
	glm::vec2 halfScreenSize{ windowSize.x / 2.f, windowSize.y / 2.f };
	float zoom{ 4.f };
	glm::mat4 projectionMatrix{ glm::ortho(
		-halfScreenSize.x / zoom, halfScreenSize.x / zoom,
		-halfScreenSize.y / zoom, halfScreenSize.y / zoom,
		-1000.0f, 1000.0f) };
	glm::mat4 viewMatrix{ camera->getViewMatrix() };
	glm::mat4 viewProjectionMatrix{ projectionMatrix * viewMatrix };

	// Render the entity sprites.
	glBindVertexArray(m_VAO);

	// Update the instance buffers.
	glBindBuffer(GL_ARRAY_BUFFER, m_posSizeVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numBoxes * sizeof(GLfloat) * 4, m_posSizeData);

	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numBoxes * sizeof(GLubyte) * 4, m_colourData);

	// Use the shader.
	m_boxShader->use();

	// Set the camera uniforms.
	m_boxShader->setVec3("cameraWorldRight", viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	m_boxShader->setVec3("cameraWorldUp", viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
	m_boxShader->setMat4("viewProjection", viewProjectionMatrix);

	// Draw the instances.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_numBoxes);
}