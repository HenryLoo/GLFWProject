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

	const float SECONDS_PER_FRAME{ 1 / 60.f };
	const int NUM_SPRITES_PER_SECOND{ 1000 };
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
	m_positionData = new GLfloat[MAX_SPRITES * 4];
	m_colourData = new GLubyte[MAX_SPRITES * 4];
	for (int i = 0; i < MAX_SPRITES; i++) {
		m_sprites[i].duration = -1.0f;
		m_sprites[i].cameraDistance = -1.0f;
	}

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
	glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// Set attribute for instance positions.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(1, 1);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

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

void SpriteRenderer::update(float deltaTime, Camera *camera)
{
	// The number of new sprites to create this frame.
	int numNewSprites = (int)(deltaTime * NUM_SPRITES_PER_SECOND);
	if (numNewSprites > (int)(SECONDS_PER_FRAME * NUM_SPRITES_PER_SECOND))
		numNewSprites = (int)(SECONDS_PER_FRAME * NUM_SPRITES_PER_SECOND);

	// Generate the new sprites with random values.
	for (int i = 0; i < numNewSprites; i++)
	{
		int spriteIndex = findUnusedSprite();
		m_sprites[spriteIndex].duration = 5.0f;
		m_sprites[spriteIndex].pos = glm::vec3(0, 0, -20.0f);

		float spread = 1.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		m_sprites[spriteIndex].speed = maindir + randomdir * spread;

		m_sprites[spriteIndex].r = rand() % 256;
		m_sprites[spriteIndex].g = rand() % 256;
		m_sprites[spriteIndex].b = rand() % 256;
		m_sprites[spriteIndex].a = 255;

		m_sprites[spriteIndex].scale = (rand() % 1000) / 2000.0f + 0.1f;
	}

	// Update sprite values for this frame.
	m_numSprites = 0;
	glm::mat4 ViewMatrix = camera->getViewMatrix();
	glm::vec3 CameraPosition = camera->getPosition();
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		Sprite &thisSprite = m_sprites[i];

		if (thisSprite.duration > 0.0f)
		{
			thisSprite.duration -= deltaTime;
			if (thisSprite.duration > 0.0f)
			{
				// Update this sprite's values.
				thisSprite.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
				thisSprite.pos += thisSprite.speed * (float)deltaTime;
				thisSprite.cameraDistance = glm::length2(thisSprite.pos - CameraPosition);

				// Update the data buffers with this sprite's values.
				m_positionData[4 * m_numSprites + 0] = thisSprite.pos.x;
				m_positionData[4 * m_numSprites + 1] = thisSprite.pos.y;
				m_positionData[4 * m_numSprites + 2] = thisSprite.pos.z;

				m_positionData[4 * m_numSprites + 3] = thisSprite.scale;

				m_colourData[4 * m_numSprites + 0] = thisSprite.r;
				m_colourData[4 * m_numSprites + 1] = thisSprite.g;
				m_colourData[4 * m_numSprites + 2] = thisSprite.b;
				m_colourData[4 * m_numSprites + 3] = thisSprite.a;
			}
			else
			{
				// Set distance to camera to be minimum value so that
				// sortSprites() will place it at the back of the array.
				thisSprite.cameraDistance = -1.0f;
			}

			m_numSprites++;
		}
	}

	sortSprites();
}

void SpriteRenderer::render(Camera *camera, float aspectRatio)
{
	glBindVertexArray(m_VAO);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),
		1024 / 768.f, 0.1f, 100.0f);
	glm::mat4 viewMatrix = camera->getViewMatrix();
	glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	// Update the instance buffers.
	glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numSprites * sizeof(GLfloat) * 4, m_positionData);

	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
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

int SpriteRenderer::findUnusedSprite() {

	for (int i = m_lastUsedSprite; i < MAX_SPRITES; i++)
	{
		if (m_sprites[i].duration < 0)
		{
			m_lastUsedSprite = i;
			return i;
		}
	}

	for (int i = 0; i < m_lastUsedSprite; i++)
	{
		if (m_sprites[i].duration < 0)
		{
			m_lastUsedSprite = i;
			return i;
		}
	}

	// No available sprite, so just overwrite the first one.
	return 0;
}

void SpriteRenderer::sortSprites()
{
	std::sort(&m_sprites[0], &m_sprites[MAX_SPRITES]);
}