#pragma once
#ifndef SpriteRenderer_H
#define SpriteRenderer_H

#include "Shader.h"
#include "Texture.h"
#include "GameComponent.h"
#include "GameEngine.h"

#include <memory>

// A paired data structure consisting of the physics and sprite components.
// This is used for sorting both components simultaneously with respect 
// to camera distance.
struct Sprite
{
	// Physics components.
	glm::vec3 pos;
	float scale;

	// Sprite components.
	unsigned char r, g, b, a;
	int frameIndex{ 0 };
	float cameraDistance{ -1 };

	bool operator<(const Sprite &that) const
	{
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
};

class Camera;

class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	// Update the array of sprites to prepare for rendering.
	// The resulting array has not yet been ordered with respect to
	// camera distance.
	void updateSprites(const GameComponent::Physics &physics, 
		const GameComponent::Sprite &sprite);

	// Set the number of sprites to render to 0.
	// This should be called every frame from the update loop, so that
	// the proper number of sprites can be recalculated.
	void resetNumSprites();

	// Increase the number of sprites to render by 1.
	void incrementNumSprites();

	// Sort the array of sprites with respect to camera distance and then
	// populate the data buffers with their values for the GPU.
	void updateData();

	// Render all queued meshes.
	void render(Camera *camera, float aspectRatio);

private:
	// Shaders to render with.
	std::unique_ptr<Shader> m_spriteShader;

	// Data to send to the GPU.
	GLfloat *m_positionData;
	GLubyte *m_colourData;
	GLfloat *m_texCoordsData;

	// The vertex array object and vertex buffer object for sprite instances.
	GLuint m_VAO, m_verticesVBO, m_positionVBO, m_colourVBO, m_texCoordsVBO;

	// Hold all the sprites to render.
	Sprite m_sprites[GameEngine::MAX_ENTITIES];

	// The number of sprites to render for the current frame.
	int m_numSprites{ 0 };

	// TODO: remove this later for a more flexible implementation.
	std::unique_ptr<Texture> m_texture;
};

#endif