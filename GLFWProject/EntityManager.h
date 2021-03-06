#pragma once
#ifndef EntityManager_H
#define EntityManager_H

#include "GameComponent.h"
#include "CollisionBroadPhase.h"
#include "DebugSystem.h"
#include "EntityConstants.h"

#include <json/include/nlohmann/json_fwd.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <memory>
#include <vector>

class GameEngine;
class AssetLoader;
class Sound;
class InputManager;
class SpriteRenderer;
class UIRenderer;
class TextRenderer;
class Prefab;

namespace SoLoud
{
	class Soloud;
}

class EntityManager
{
public:
	EntityManager(GameEngine *game, AssetLoader *assetLoader, 
		InputManager *inputManager, SpriteRenderer *sRenderer, 
		UIRenderer *uRenderer, SoLoud::Soloud &soundEngine);
	~EntityManager();

	// Update all appropriate values for the game loop's current iteration.
	void update(float deltaTime, AssetLoader *assetLoader, 
		UIRenderer *uRenderer, TextRenderer *tRenderer);
	void updateDebug(float deltaTime);

	// Flag an entity for deletion.
	void deleteEntity(int id);

	// Getter functions.
	int getPlayerId() const;
	glm::vec3 getPlayerPos() const;
	const std::vector<std::pair<AABBSource, AABBSource>> &getCollisions() const;
	int getNumEntities() const;

	// Create an effect.
	void createEffect(const std::string &type, glm::vec3 pos, glm::vec2 scale,
		unsigned char r = 255, unsigned char g = 255, unsigned char b = 255,
		unsigned char a = 255, float rotation = 0.f);

	// Create an entity from a prefab at a given position and return its id.
	int createEntity(std::string prefabName, glm::vec2 pos = glm::vec2(0.f));

	// Initialize the Lua state and bind appropriate functions.
	void initLua();

	// Perform linear interpolation between the previous and the current
	// physics component values.
	void lerpPhysics(float amount);

private:
	// Create a new entity, given a list of component types.
	// Return the new entity's id.
	int createEntity(std::vector<GameComponent::ComponentType> types);

	// Component initializers. 
	// Player and enemy initializers return their respective script names.
	void initializeSprite(int entityId, const nlohmann::json &json);
	std::string initializePlayer(const nlohmann::json &json);
	void initializeCollision(int entityId, const nlohmann::json &json);
	void initializeWeapon(int entityId, const nlohmann::json &json);
	void initializeAttack(int entityId, const nlohmann::json &json);
	void initializeCharacter(int entityId, const nlohmann::json &json);
	std::string initializeEnemy(int entityId, const nlohmann::json &json);

	// Delete all flagged entities.
	void deleteFlaggedEntities();

	// The Lua state to use for defining entities.
	sol::state m_lua;

	// Hold bitmasks that determines each entity's components.
	// If the value is 0, then the entity is dead.
	std::vector<unsigned long> m_entities;

	// Hold the ids of all entities to delete at the end of the game loop.
	std::vector<int> m_entitiesToDelete;

	// Hold the number of alive entities.
	int m_numEntities{ 0 };

	// Hold the player's entity id.
	int m_playerId{ EntityConstants::PLAYER_NOT_SET };

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

	// Hold the physics component values from the previous frame.
	std::vector<GameComponent::Physics> m_prevPhysics;

	// Hold all game systems.
	std::vector<std::unique_ptr<GameSystem>> m_gameSystems;
	std::unique_ptr<DebugSystem> m_debugSystem;

	// Collision detection.
	std::vector<std::pair<AABBSource, AABBSource>> m_collisions;
	std::unique_ptr<CollisionBroadPhase> m_broadPhase;

	// Hold pointers and references to other core systems.
	AssetLoader *m_assetLoader;
	InputManager *m_inputManager;
	SoLoud::Soloud &m_soundEngine;

	// TODO: remove this later for a more flexible implementation.
	std::shared_ptr<SpriteSheet> m_effectsTexture;
};

#endif