#include "SpriteRenderer.h"

#include "EntityConstants.h"
#include "Room.h"
#include "AssetLoader.h"
#include "SpriteSheet.h"
#include "Shader.h"
#include "Camera.h"

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

	const GLfloat ROOM_VERTICES[] = {
		0.f, 0.f,
		1.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
	};

	// Vertex attributes for a quad that fills the entire screen in 
	// Normalized Device Coordinates.
	const GLfloat SCREEN_VERTICES[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
}

SpriteRenderer::SpriteRenderer(AssetLoader *assetLoader)
{
	// Configure GL settings.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS);

	// Load resources.
	m_spriteShader = assetLoader->load<Shader>("sprite");
	m_roomShader = assetLoader->load<Shader>("room");
	m_tileset = assetLoader->load<Texture>("tileset");

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
	glVertexAttribDivisorARB(0, 0);

	// Create the VBO for instance colours.
	// Each vertex holds 4 values: r, g, b, a.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Set attribute for instance colours.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)0);
	glVertexAttribDivisorARB(1, 1);

	// Create the VBO for instance texture coordinates.
	// Each vertex holds 4 values: u, v of the top-left texture point, 
	// and the normalized width, height of the clip size.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// Set attribute for instance texture coordinates.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisorARB(2, 1);

	// Create the VBO for instance model view matrices.
	// Initialize with an empty buffer and update its values in the game loop.
	glGenBuffers(1, &m_modelViewsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_modelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Set attributes for instance model view matrices.
	// A mat4 is equivalent to 4 vec4's.
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
	glVertexAttribDivisorARB(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
	glVertexAttribDivisorARB(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisorARB(6, 1);

	// Create the vertex array object and bind to it.
	// All subsequent VBO configurations will be saved for this VAO.
	glGenVertexArrays(1, &m_roomVAO);
	glBindVertexArray(m_roomVAO);

	// Define the vertex data for a room quad, for the VBO.
	glGenBuffers(1, &m_roomVertsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_roomVertsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ROOM_VERTICES), ROOM_VERTICES, GL_STATIC_DRAW);

	// Set attribute for vertices.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// Create the vertex array and buffer objects for post-processing.
	glGenVertexArrays(1, &m_screenVAO);
	glGenBuffers(1, &m_screenVBO);
	glBindVertexArray(m_screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES), SCREEN_VERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteBuffers(1, &m_verticesVBO);
	glDeleteBuffers(1, &m_colourVBO);
	glDeleteBuffers(1, &m_texCoordsVBO);
	glDeleteBuffers(1, &m_modelViewsVBO);
	glDeleteVertexArrays(1, &m_VAO);

	glDeleteBuffers(1, &m_roomVertsVBO);
	glDeleteVertexArrays(1, &m_roomVAO);

	glDeleteBuffers(1, &m_screenVAO);
	glDeleteBuffers(1, &m_screenVBO);
	glDeleteFramebuffers(1, &m_screenFBO);
	glDeleteTextures(1, &m_screenTexture);
	glDeleteRenderbuffers(1, &m_screenRBO);
}

void SpriteRenderer::addSprite(const GameComponent::Physics &physics, 
	const GameComponent::Sprite &sprite)
{
	const std::string spriteName{ sprite.spriteSheet->getName() };
	if (spriteName.empty())
	{
		std::cout << "SpriteRenderer::addSprite: sprite name was empty" << std::endl;
		return;
	}

	auto it{ m_spriteData.find(spriteName) };
	if (it == m_spriteData.end())
	{
		// Sprite sheet has not been added yet, so keep track of its insertion
		// order.
		m_spriteOrder.push_back(spriteName);

		// Insert new sprite data for this sprite sheet.
		SpriteData data;
		data.spriteSheet = sprite.spriteSheet.get();
		addSpriteData(data, physics, sprite);
		m_spriteData.insert({ spriteName, data });
	}
	else
	{
		// Sprite sheet was already added, so just push the vertex data for
		// this instance.
		SpriteData &data{ it->second };
		addSpriteData(data, physics, sprite);
	}
}

void SpriteRenderer::addSpriteData(SpriteData &data, const GameComponent::Physics &physics,
	const GameComponent::Sprite &sprite) const
{
	data.colours.push_back(sprite.r);
	data.colours.push_back(sprite.g);
	data.colours.push_back(sprite.b);
	data.colours.push_back(sprite.a);

	const glm::vec2 &texSize{ sprite.spriteSheet->getSize() };
	const SpriteSheet::SpriteClip& thisClip{ sprite.currentSprite.clips[sprite.currentFrame] };
	const glm::vec2 &clipSize{ thisClip.clipSize };

	data.texCoords.push_back(thisClip.topLeft.x / texSize.x);
	data.texCoords.push_back(1 - thisClip.topLeft.y / texSize.y);
	data.texCoords.push_back(clipSize.x / texSize.x);
	data.texCoords.push_back(clipSize.y / texSize.y);

	// Construct model view matrix for this sprite.
	glm::mat4 modelMatrix{ glm::mat4(1.0f) };

	// Apply translation.
	glm::vec3 translation{
		physics.pos.x + physics.scale.x * thisClip.offset.x,
		physics.pos.y + physics.scale.y * thisClip.offset.y,
		physics.pos.z
	};
	modelMatrix = glm::translate(modelMatrix, translation);

	// Apply scaling.
	glm::vec3 scale{
		physics.scale.x * clipSize.x,
		physics.scale.y * clipSize.y,
		1.f
	};
	modelMatrix = glm::scale(modelMatrix, scale);

	// Apply rotation.
	modelMatrix = glm::rotate(modelMatrix, physics.rotation, glm::vec3(0.f, 0.f, 1.f));

	// Left-multiply by view matrix to get model view matrix.
	glm::mat4 modelViewMatrix{ m_viewMatrix * modelMatrix };

	data.modelViews.push_back(modelViewMatrix);
	
	data.numSprites++;
}

void SpriteRenderer::resetData()
{
	// Clear sprite data.
	m_spriteData.clear();
	m_spriteOrder.clear();
}

void SpriteRenderer::render(Camera *camera, Room *room = nullptr,
	Shader *postShader)
{
	// If using post-processing shader, bind to the framebuffer object 
	// before drawing. This will draw the scene on the attached texture.
	bool isUsingPostShader{ m_screenFBO && postShader != nullptr };
	if (isUsingPostShader)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Orthographic projection with origin of the coordinate space defined at
	// the center of the screen. Negative y-axis points down.
	glm::mat4 projectionMatrix{ getProjectionMatrix(camera->getZoom()) };

	// Render the background room tiles first.
	// This should only render the tiles that are visible in the camera.
	if (room != nullptr)
	{
		glBindVertexArray(m_roomVAO);
		m_roomShader->use();

		glActiveTexture(GL_TEXTURE0);
		m_tileset->bind();
		glActiveTexture(GL_TEXTURE1);
		room->getTileSprites()->bind();

		// Set the room uniforms.
		m_roomShader->setInt("tilesetTexture", 0);
		m_roomShader->setInt("layoutTexture", 1);
		int tileSize{ Room::TILE_SIZE };
		int textureSize{ m_tileset->getSize().x };
		int numTilesInTilesetRow{ textureSize / tileSize };
		m_roomShader->setVec3("tileSetVals", glm::vec3(tileSize, numTilesInTilesetRow, textureSize));
		m_roomShader->setVec2("mapSizeInTiles", room->getSize());

		// Set the camera uniforms.
		glm::mat4 modelMatrix{ glm::mat4(1.f) };
		glm::ivec2 roomSize{ room->getSize() * tileSize };
		modelMatrix = glm::scale(modelMatrix, glm::vec3(roomSize.x, roomSize.y, 1.f));
		m_roomShader->setMat4("mvp", projectionMatrix * m_viewMatrix * modelMatrix);

		// Draw the room.
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	// Render the entity sprites.
	glBindVertexArray(m_VAO);

	// Update the instance buffers.
	glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(EntityConstants::MAX_ENTITIES) * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_modelViewsVBO);
	glBufferData(GL_ARRAY_BUFFER, EntityConstants::MAX_ENTITIES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	// Use the shader.
	m_spriteShader->use();

	// Bind to the texture at texture unit 2 and set the shader's sampler to this.
	glActiveTexture(GL_TEXTURE2);
	m_spriteShader->setInt("textureSampler", 2);

	// Set the camera uniforms.
	m_spriteShader->setMat4("projection", projectionMatrix);

	// Iterate through each spritesheet type in order of their insertion.
	// Bind to the spritesheet texture, and then call draw.
	for (const std::string &sprite : m_spriteOrder)
	{
		auto it{ m_spriteData.find(sprite) };
		if (it != m_spriteData.end())
		{
			SpriteData &data{ it->second };

			glBindBuffer(GL_ARRAY_BUFFER, m_colourVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, data.numSprites * sizeof(GLubyte) * 4, &data.colours[0]);

			glBindBuffer(GL_ARRAY_BUFFER, m_texCoordsVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, data.numSprites * sizeof(GLfloat) * 4, &data.texCoords[0]);

			glBindBuffer(GL_ARRAY_BUFFER, m_modelViewsVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, data.numSprites * sizeof(glm::mat4), &data.modelViews[0]);

			// Draw the instances.
			data.spriteSheet->bind();
			m_spriteShader->setVec2("textureSize", data.spriteSheet->getSize());
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, data.numSprites);
		}
		else
		{
			std::cout << "SpriteRenderer::render: Could not find added sprite!" << std::endl;
		}
	}

	if (isUsingPostShader)
	{
		// Unbind and use the normal frame buffer to render to the main window.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Set the texture sampler to use texture id 2, to match the above 
		// sprite shader.
		postShader->use();
		postShader->setInt("screenTexture", 2);

		glBindVertexArray(m_screenVAO);

		// Bind to the texture that the scene was drawn on, and render it
		// onto a screen-wide quad.
		glBindTexture(GL_TEXTURE_2D, m_screenTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

void SpriteRenderer::createFramebuffer(glm::ivec2 windowSize)
{
	// Delete any existing old buffers.
	if (m_screenFBO) glDeleteFramebuffers(1, &m_screenFBO);
	if (m_screenTexture) glDeleteTextures(1, &m_screenTexture);
	if (m_screenRBO) glDeleteRenderbuffers(1, &m_screenRBO);

	// Create the frame buffer and bind to it.
	glGenFramebuffers(1, &m_screenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);

	// Attach a texture to the frame buffer.
	// All rendering commands will write to this texture.
	glGenTextures(1, &m_screenTexture);
	glBindTexture(GL_TEXTURE_2D, m_screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowSize.x, windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screenTexture, 0);

	// Create the render buffer for depth and stencil testing.
	glGenRenderbuffers(1, &m_screenRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_screenRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSize.x, windowSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_screenRBO);

	// Check if the framebuffer is complete.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "SpriteRenderer::createFramebuffer: Framebuffer not complete." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}