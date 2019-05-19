#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"
#include "GameComponent.h"
#include "SpriteSheet.h"
#include "Room.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

class Renderer;
class SpriteRenderer;
class InputManager;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start(SpriteRenderer *renderer, InputManager *input);

	// Update the camera to look at a position on the screen.
	void updateCameraLook(glm::vec2 screenPos);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// Create a new entity, given a list of component types.
	// Return the new entity's id.
	int createEntity(std::vector<GameComponent::ComponentType> types);

	// Flag an entity for deletion.
	void deleteEntity(int id);

	// The maximum number of entities supported by the game.
	static const int MAX_ENTITIES{ 100000 };

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput();

	// Update all appropriate values for the game loop's current iteration.
	void update(SpriteRenderer *renderer, InputManager *input);

	// Render all appropriate visuals for the game loop's current iteration.
	void render(SpriteRenderer *renderer);

	// Delete all flagged entities.
	void deleteFlaggedEntities();

	// TODO: test function for generating entities, remove this later.
	void createNewEntities();

	void createPlayer();

	// The window to render to.
	GLFWwindow *m_window{ nullptr };
	glm::ivec2 m_windowSize;

	// The time between the current and last frame.
	float m_deltaTime{ 0.0f };

	// The time of the last frame.
	float m_lastFrame{ 0.0f };

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;

	// Flag for if the window size was changed.
	bool m_hasNewWindowSize{ true };

	// Hold bitmasks that determines each entity's components.
	// If the value is 0, then the entity is dead.
	unsigned long m_entities[MAX_ENTITIES];

	// Hold the player's entity id.
	int m_playerId;

	// Hold components for each entity.
	GameComponent::Physics m_compPhysics[MAX_ENTITIES];
	GameComponent::Sprite m_compSprites[MAX_ENTITIES];
	GameComponent::Player m_compPlayer; // Only one player.

	// Hold the number of alive entities.
	int m_numEntities{ 0 };

	// Hold the ids of all entities to delete at the end of the game loop.
	std::vector<int> m_entitiesToDelete;

	// TODO: remove this later for a more flexible implementation.
	std::unique_ptr<SpriteSheet> m_texture;
	std::unique_ptr<Room> m_currentRoom;
};

#endif