#include "SpriteRenderer.h"

#include "Camera.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <iostream>

namespace
{
	const GLfloat QUAD_VERTICES[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
}

SpriteRenderer::SpriteRenderer()
{
	// Configure GL settings.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	// TODO: replace these hardcoded resources.
	m_spriteShader = std::make_unique<Shader>("sprite.vs", "sprite.fs");
	m_texture = std::make_unique<Texture>("serah_idle.png");

	// Prepare the data buffers.
	m_positionData = new GLfloat[GameEngine::MAX_ENTITIES * 4];
	m_colourData = new GLubyte[GameEngine::MAX_ENTITIES * 4];

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(0, 0);

	// Create the VBO for instance positions.
	// Each vertex holds 4 values: x, y, z, scale.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
	glBufferData(GL_ARRAY_BUFFER, GameEngine::MAX_ENTITIES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// Set attribute for instance positions.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, GameEngine::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)0);
	glVertexAttribDivisor(2, 1);
}

SpriteRenderer::~SpriteRenderer()
{
	delete[] m_positionData;
	delete[] m_colourData;

	glDeleteBuffers(1, &m_colourVBO);
	glDeleteBuffers(1, &m_positionVBO);
	glDeleteBuffers(1, &m_verticesVBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void SpriteRenderer::updateSprites(const GameComponent::Physics &physics, 
	const GameComponent::Sprite &sprite)
{
	Sprite &spr{ m_sprites[m_numSprites] };
	spr.pos = physics.pos;
	spr.scale = physics.scale;
	spr.r = sprite.r;
	spr.g = sprite.g;
	spr.b = sprite.b;
	spr.a = sprite.a;
	spr.cameraDistance = sprite.cameraDistance;
}

void SpriteRenderer::resetNumSprites()
{
	m_numSprites = 0;
}

void SpriteRenderer::incrementNumSprites()
{
	m_numSprites++;
}

void SpriteRenderer::updateData()
{
	// Sort the sprites by camera distance to maintain proper
	// draw order.
	std::sort(&m_sprites[0], &m_sprites[GameEngine::MAX_ENTITIES]);

	// Update the data buffers with these components' values.
	for (int i = 0; i < m_numSprites; i++)
	{
		Sprite &spr{ m_sprites[i] };

		m_positionData[4 * i + 0] = spr.pos.x;
		m_positionData[4 * i + 1] = spr.pos.y;
		m_positionData[4 * i + 2] = spr.pos.z;
		m_positionData[4 * i + 3] = spr.scale;

		m_colourData[4 * i + 0] = spr.r;
		m_colourData[4 * i + 1] = spr.g;
		m_colourData[4 * i + 2] = spr.b;
		m_colourData[4 * i + 3] = spr.a;
	}
}

void SpriteRenderer::render(Camera *camera, float aspectRatio)
{
	glBindVertexArray(m_VAO);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projectionMatrix{ glm::perspective(glm::radians(45.0f),
		1024 / 768.f, 0.1f, 100.0f) };
	glm::mat4 viewMatrix{ camera->getViewMatrix() };
	glm::mat4 viewProjectionMatrix{ projectionMatrix * viewMatrix };

	// Update the instance buffers.
	glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
	glBufferData(GL_ARRAY_BUFFER, GameEngine::MAX_ENTITIES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numSprites * sizeof(GLfloat) * 4, m_positionData);

	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, GameEngine::MAX_ENTITIES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numSprites * sizeof(GLubyte) * 4, m_colourData);

	// Use the shader.
	m_spriteShader->use();

	// Bind to the texture at texture unit 0 and set the shader's sampler to this.
	glActiveTexture(GL_TEXTURE0);
	m_texture->bind();
	m_spriteShader->setInt("textureSampler", 0);

	// Set the camera uniforms.
	m_spriteShader->setVec3("cameraWorldRight", viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	m_spriteShader->setVec3("cameraWorldUp", viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
	m_spriteShader->setMat4("viewProjection", viewProjectionMatrix);

	// Draw the instances.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_numSprites);
}