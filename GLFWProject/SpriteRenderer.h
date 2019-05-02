#pragma once
#ifndef SpriteRenderer_H
#define SpriteRenderer_H

#include "Shader.h"
#include "Texture.h"

#include <memory>

struct Sprite
{
	// This sprite's position.
	glm::vec3 pos;
	
	// This sprite's movement speed.
	glm::vec3 speed;

	// This sprite's colour.
	unsigned char r, g, b, a;

	// This sprite's physical attributes.
	float scale, angle, weight;

	// Determines how long this sprite has left to live, in seconds.
	// If the value is < 0, then this sprite is dead.
	float duration;

	// Distance from the sprite to the camera.
	float cameraDistance;

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

	// Update all renderer values.
	void update(float deltaTime, Camera *camera);

	// Render all queued meshes.
	void render(Camera *camera, float aspectRatio);

	// The maximum number of sprites that the renderer can draw.
	static const int MAX_SPRITES{ 100000 };

private:
	// Find the index of an unused sprite.
	int findUnusedSprite();

	// Sort the sprites by distance to camera, so that sprites furthest from 
	// the camera are drawn first.
	void sortSprites();

	// Shaders to render with.
	std::unique_ptr<Shader> m_spriteShader;

	// Data to send to the GPU.
	GLfloat *m_positionData;
	GLubyte *m_colourData;

	// The vertex array object and vertex buffer object for sprite instances.
	GLuint m_VAO, m_verticesVBO, m_positionVBO, m_colourVBO;

	// Hold all the sprites to render.
	Sprite m_sprites[MAX_SPRITES];

	// The index of the last used sprite in m_sprites.
	int m_lastUsedSprite{ 0 };

	// The number of sprites to render for the current frame.
	int m_numSprites{ 0 };

	// TODO: remove this later for a more flexible implementation.
	std::unique_ptr<Texture> m_texture;
};

#endif