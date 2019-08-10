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

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance model view matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_modelViewsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_modelViewsVBO);
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

UIRenderer::~UIRenderer()
{
	glDeleteBuffers(1, &m_verticesVBO);
	glDeleteBuffers(1, &m_colourVBO);
	glDeleteBuffers(1, &m_modelViewsVBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void UIRenderer::addBox(const GameComponent::Physics &physics,
	const AABB &aabb,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	// Push colour data.
	m_colourData.push_back(r);
	m_colourData.push_back(g);
	m_colourData.push_back(b);
	m_colourData.push_back(a);

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

	// Apply rotation.
	modelMatrix = glm::rotate(modelMatrix, physics.rotation, glm::vec3(0.f, 0.f, 1.f));

	// Left-multiply by view matrix to get model view matrix.
	glm::mat4 modelViewMatrix{ m_viewMatrix * modelMatrix };

	m_modelViewsData.push_back(modelViewMatrix);
}

void UIRenderer::resetNumBoxes()
{
	// Clear data.
	m_colourData.clear();
	m_modelViewsData.clear();
}

void UIRenderer::update(const glm::mat4& viewMatrix)
{
	m_viewMatrix = viewMatrix;
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

	// Render the entity sprites.
	glBindVertexArray(m_VAO);

	// Update the instance buffers.
	size_t numBoxes{ m_modelViewsData.size() };
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numBoxes * sizeof(GLubyte) * 4, &m_colourData[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_modelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numBoxes * sizeof(glm::mat4), &m_modelViewsData[0]);

	// Use the shader.
	m_boxShader->use();

	// Set the camera uniforms.
	m_boxShader->setMat4("projection", projectionMatrix);

	// Draw the instances.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numBoxes);
}