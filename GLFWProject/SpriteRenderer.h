#pragma once
#ifndef SpriteRenderer_H
#define SpriteRenderer_H

#include "Renderer.h"
#include "GameComponent.h"
#include "Room.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

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

class Shader;
class SpriteSheet;

struct SpriteData
{
	SpriteSheet *spriteSheet{ nullptr };

	// The number of entities using this sprite.
	int numSprites{ 0 };

	// Data to send to the GPU.
	std::vector<GLubyte> colours;
	std::vector<GLfloat> texCoords;
	std::vector<glm::mat4> models;
};

class Camera;
class AssetLoader;

class SpriteRenderer : public Renderer
{
public:
	SpriteRenderer(AssetLoader *assetLoader);
	~SpriteRenderer();

	// Add sprite data to the array of sprites to prepare for rendering.
	// The resulting array has not yet been ordered with respect to
	// camera distance.
	void addSprite(const GameComponent::Physics &physics, 
		const GameComponent::Sprite &sprite);
	void addSprite(const RoomData::Layer &layer,
		std::shared_ptr<SpriteSheet> spriteSheet);

	// Set the number of sprites to render to 0.
	// This should be called every frame from the update loop, so that
	// the proper number of sprites can be recalculated.
	virtual void resetData();

	// Render the current room and all queued sprites.
	void render(Camera* camera, Room *room, Shader *postShader = nullptr);

	// Create the frame buffer for post-processing effects.
	void createFramebuffer(glm::ivec2 windowSize);

private:
	// Add vertex values to SpriteData.
	void addSpriteData(SpriteData &data, const GameComponent::Physics &physics,
		const GameComponent::Sprite &sprite) const;

	// Shaders to render with.
	std::shared_ptr<Shader> m_spriteShader;
	std::shared_ptr<Shader> m_roomShader;
	std::shared_ptr<Shader> m_bgShader;

	// The vertex array object and vertex buffer object for sprite instances.
	GLuint m_VAO, m_verticesVBO, m_colourVBO, m_texCoordsVBO, m_modelsVBO;

	// The vertex array object and vertex buffer object for the room.
	GLuint m_roomVAO, m_roomVertsVBO;

	// The framebuffer used for post-processing.
	GLuint m_screenVAO, m_screenVBO, m_screenFBO, m_screenTexture, m_screenRBO;

	// Hold the pointer to the map tileset.
	std::shared_ptr<Texture> m_tileset;

	// Map texture name to its data.
	// This groups all vertex data with common textures so they can be
	// instanced together.
	std::unordered_map<std::string, SpriteData> m_spriteData;

	// Holds texture names, in order of insertion into m_spriteData.
	std::vector<std::string> m_spriteOrder;
};

#endif