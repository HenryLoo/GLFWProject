#pragma once
#ifndef EntityManager_H
#define EntityManager_H

#include "GameComponent.h"
#include "CollisionBroadPhase.h"
#include "DebugSystem.h"

#include <memory>
#include <vector>

class GameEngine;
class AssetLoader;
class Sound;
class InputManager;
class SpriteRenderer;
class UIRenderer;

class EntityManager
{
public:
	EntityManager(GameEngine *game, AssetLoader *assetLoader, 
		InputManager *inputManager, SpriteRenderer *sRenderer, 
		UIRenderer *uRenderer);
	~EntityManager();

	// Update all appropriate values for the game loop's current iteration.
	void update(float deltaTime, bool isDebugMode);

	// Create a new entity, given a list of component types.
	// Return the new entity's id.
	int createEntity(std::vector<GameComponent::ComponentType> types);

	// Flag an entity for deletion.
	void deleteEntity(int id);

	// Getter functions.
	int getPlayerId() const;
	glm::vec3 getPlayerPos() const;
	const std::vector<std::pair<AABBSource, AABBSource>> &getCollisions() const;

	// Create an effect.
	void createEffect(const std::string &type, glm::vec3 pos, glm::vec2 scale,
		unsigned char r = 255, unsigned char g = 255, unsigned char b = 255,
		unsigned char a = 255, float rotation = 0.f);

private:
	// Delete all flagged entities.
	void deleteFlaggedEntities();

	// TODO: test function for generating entities, remove this later.
	//void createNewEntities();
	void createPlayer();
	void createEnemy();

	// Hold bitmasks that determines each entity's components.
	// If the value is 0, then the entity is dead.
	std::vector<unsigned long> m_entities;

	// Hold the ids of all entities to delete at the end of the game loop.
	std::vector<int> m_entitiesToDelete;

	// Hold the number of alive entities.
	int m_numEntities{ 0 };

	// Hold the player's entity id.
	int m_playerId;

	// Hold this frame's deltaTime.
	float m_deltaTime;

	// Hold components for each entity.
	std::vector<GameComponent::Physics> m_compPhysics;
	std::vector<GameComponent::Sprite> m_compSprites;
	GameComponent::Player m_compPlayer; // Only one player.
	std::vector<GameComponent::Collision> m_compCollisions;
	std::vector<GameComponent::Weapon> m_compWeapons;
	std::vector<GameComponent::Attack> m_compAttacks;
	std::vector<GameComponent::Enemy> m_compEnemies;
	std::vector<GameComponent::Character> m_compCharacters;

	// Hold all game systems.
	std::vector<std::unique_ptr<GameSystem>> m_gameSystems;
	std::unique_ptr<DebugSystem> m_debugSystem;

	// Collision detection.
	std::vector<std::pair<AABBSource, AABBSource>> m_collisions;
	std::unique_ptr<CollisionBroadPhase> m_broadPhase;

	// Hold pointers to other core systems.
	AssetLoader *m_assetLoader;
	InputManager *m_inputManager;

	// TODO: remove this later for a more flexible implementation.
	std::shared_ptr<SpriteSheet> m_playerTexture;
	std::shared_ptr<SpriteSheet> m_swordTexture;
	std::shared_ptr<SpriteSheet> m_enemyTexture;
	std::shared_ptr<SpriteSheet> m_effectsTexture;
	std::shared_ptr<Sound> m_jumpSound;
};

#endif