#pragma once
#ifndef SpriteRenderer_H
#define SpriteRenderer_H

#include "Shader.h"
#include "SpriteSheet.h"
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
	glm::vec2 scale;
	float rotation;

	// Sprite components.
	unsigned char r, g, b, a;
	SpriteSheet *spriteSheet;
	int frameIndex{ 0 };
	float cameraDistance{ -1 };

	bool operator<(const Sprite &that) const
	{
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
};

class Camera;
class Room;

class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	// Add sprite data to the array of sprites to prepare for rendering.
	// The resulting array has not yet been ordered with respect to
	// camera distance.
	void addSprite(const GameComponent::Physics &physics, 
		const GameComponent::Sprite &sprite);

	// Set the number of sprites to render to 0.
	// This should be called every frame from the update loop, so that
	// the proper number of sprites can be recalculated.
	void resetNumSprites();

	// Sort the array of sprites with respect to camera distance and then
	// populate the data buffers with their values for the GPU.
	//void updateData();

	// Render the current room and all queued sprites.
	void render(Camera *camera, glm::ivec2 windowSize, Room *room);

private:
	// Shaders to render with.
	std::unique_ptr<Shader> m_spriteShader;
	std::unique_ptr<Shader> m_roomShader;

	// Data to send to the GPU.
	GLfloat *m_positionData;
	GLubyte *m_colourData;
	GLfloat *m_texCoordsData;
	GLfloat *m_transformData;

	// The vertex array object and vertex buffer object for sprite instances.
	GLuint m_VAO, m_verticesVBO, m_positionVBO, m_colourVBO, 
		m_texCoordsVBO, m_transformVBO;

	// Hold all the sprites to render.
	//Sprite m_sprites[GameEngine::MAX_ENTITIES];

	// The number of sprites to render for the current frame.
	int m_numSprites{ 0 };

	// The vertex array object and vertex buffer object for the room.
	GLuint m_roomVAO, m_roomVertsVBO;

	// Hold the pointer to the map tileset.
	std::unique_ptr<SpriteSheet> m_tileset;

	SpriteSheet *m_playerSheet{ nullptr };
};

#endif