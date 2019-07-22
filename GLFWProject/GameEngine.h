#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"
#include "GameComponent.h"
#include "SpriteSheet.h"
#include "Room.h"
#include "CollisionBroadPhase.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

class Renderer;
class SpriteRenderer;
class UIRenderer;
class InputManager;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start(SpriteRenderer *renderer, InputManager *input, UIRenderer *uRenderer);

	// Update the camera to look at a position on the screen.
	void updateCameraLook(glm::vec2 screenPos);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// Create a new entity, given a list of component types.
	// Return the new entity's id.
	int createEntity(std::vector<GameComponent::ComponentType> types);

	// Flag an entity for deletion.
	void deleteEntity(int id);

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput(InputManager *input);

	// Update all appropriate values for the game loop's current iteration.
	void update(SpriteRenderer *sRenderer, InputManager *input, UIRenderer *uRenderer);

	// Render all appropriate visuals for the game loop's current iteration.
	void render(SpriteRenderer *sRenderer, UIRenderer *uRenderer);

	// Delete all flagged entities.
	void deleteFlaggedEntities();

	// TODO: test function for generating entities, remove this later.
	void createNewEntities();

	void createPlayer();
	void createEnemy();

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
	std::vector<unsigned long> m_entities;

	// Hold the player's entity id.
	int m_playerId;

	// Hold components for each entity.
	std::vector<GameComponent::Physics> m_compPhysics;
	std::vector<GameComponent::Sprite> m_compSprites;
	GameComponent::Player m_compPlayer; // Only one player.
	std::vector<GameComponent::Collision> m_compCollisions;
	std::vector<GameComponent::Weapon> m_compWeapons;
	std::vector<GameComponent::Attack> m_compAttacks;
	std::vector<GameComponent::Enemy> m_compEnemies;

	// Hold the number of alive entities.
	int m_numEntities{ 0 };

	// Hold the ids of all entities to delete at the end of the game loop.
	std::vector<int> m_entitiesToDelete;

	// Hold the ids of all enemy entities.
	std::vector<int> m_enemyIds;

	// TODO: remove this later for a more flexible implementation.
	std::unique_ptr<SpriteSheet> m_playerTexture;
	std::unique_ptr<SpriteSheet> m_swordTexture;
	std::unique_ptr<Room> m_currentRoom;
	std::unique_ptr<SpriteSheet> m_enemyTexture;

	// Flag for if debug mode is enabled.
	bool m_isDebugMode{ false };

	std::unique_ptr<CollisionBroadPhase> m_broadPhase;
};

#endif