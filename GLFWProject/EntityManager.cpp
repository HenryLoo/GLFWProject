#include "EntityManager.h"

#include "CharStates.h"
#include "EffectTypes.h"
#include "AssetLoader.h"
#include "SpriteSheet.h"
#include "Sound.h"
#include "InputManager.h"
#include "Prefab.h"
#include "JSONUtilities.h"
#include "UIRenderer.h"
#include "Script.h"
#include "Room.h"

#include "AttackCollisionSystem.h"
#include "AttackSystem.h"
#include "CharacterSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSystem.h"
#include "RoomCollisionSystem.h"
#include "SpriteSystem.h"
#include "EnemySystem.h"

#include <iostream>
#include <algorithm>

using json = nlohmann::json;

namespace
{
	// Component labels for prefabs.
	const std::string COMPONENT_SPRITE{ "sprite" };
	const std::string COMPONENT_PHYSICS{ "physics" };
	const std::string COMPONENT_PLAYER{ "player" };
	const std::string COMPONENT_ENEMY{ "enemy" };
	const std::string COMPONENT_COLLISION{ "collision" };
	const std::string COMPONENT_WEAPON{ "weapon" };
	const std::string COMPONENT_ATTACK{ "attack" };
	const std::string COMPONENT_CHARACTER{ "character" };
	const std::unordered_map<std::string, GameComponent::ComponentType> LABEL_TO_COMPONENT{
		{ COMPONENT_SPRITE, GameComponent::COMPONENT_SPRITE },
		{ COMPONENT_PHYSICS, GameComponent::COMPONENT_PHYSICS },
		{ COMPONENT_PLAYER, GameComponent::COMPONENT_PLAYER },
		{ COMPONENT_ENEMY, GameComponent::COMPONENT_ENEMY },
		{ COMPONENT_COLLISION, GameComponent::COMPONENT_COLLISION },
		{ COMPONENT_WEAPON, GameComponent::COMPONENT_WEAPON },
		{ COMPONENT_ATTACK, GameComponent::COMPONENT_ATTACK },
		{ COMPONENT_CHARACTER, GameComponent::COMPONENT_CHARACTER },
	};

	const std::string PROPERTY_COMPONENTS{ "components" };
	const std::string PROPERTY_SPRITESHEET{ "spritesheet" };
	const std::string PROPERTY_EVADEDURATION{ "evadeDuration" };
	const std::string PROPERTY_EVADESOUND{ "evadeSound" };
	const std::string PROPERTY_PORTRAITICON{ "portraitIcon" };
	const std::string PROPERTY_SKILLICONS{ "skillIcons" };
	const std::string PROPERTY_BOXES{ "boxes" };
	const std::string PROPERTY_HALFSIZE{ "halfSize" };
	const std::string PROPERTY_OFFSET{ "offset" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_HEALTH{ "health" };
	const std::string PROPERTY_RESOURCE{ "resource" };
	const std::string PROPERTY_MOVESPEED{ "movementSpeed" };
	const std::string PROPERTY_JUMPSPEED{ "jumpSpeed" };
	const std::string PROPERTY_SCRIPT{ "script" };
	const std::string PROPERTY_ATTACKPATTERNS{ "attackPatterns" };
	const std::string PROPERTY_TYPE{ "type" };
	const std::string PROPERTY_HITENABLED{ "hitEnabled" };
	const std::string PROPERTY_SUPERARMOUR{ "superArmour" };
	const std::string PROPERTY_START{ "start" };
	const std::string PROPERTY_NUMFRAMES{ "numFrames" };
	const std::string PROPERTY_COMBOFRAME{ "comboFrame" };
	const std::string PROPERTY_ATTACKSOUND{ "attackSound" };
	const std::string PROPERTY_HITSOUND{ "hitSound" };
	const std::string PROPERTY_HITEFFECT{ "hitEffect" };
	const std::string PROPERTY_COOLDOWN{ "cooldown" };
	const std::string PROPERTY_DAMAGE{ "damage" };
	const std::string PROPERTY_KNOCKBACK{ "knockback" };
	const std::string PROPERTY_HITSTUN{ "hitStun" };
	const std::string PROPERTY_TARGETRANGE{ "targetRange" };
	const std::string PROPERTY_ATTACKRANGE{ "attackRange" };
	const std::string PROPERTY_ACTIONDURATION{ "actionDuration" };
	const std::string PROPERTY_ATTACKDURATION{ "attackDuration" };

	const std::string CHAR_STATES_SCRIPT{ "char_states" };
	const std::string INIT_CHAR_STATES{ "initCharStates" };
	const std::string PLAYER_STATES_SCRIPT{ "player_states" };
	const std::string INIT_PLAYER_STATES{ "initPlayerStates" };
	const std::string INIT_STATES{ "initStates" };
}

EntityManager::EntityManager(GameEngine *game, AssetLoader *assetLoader,
	InputManager *inputManager, SpriteRenderer *sRenderer,
	UIRenderer *uRenderer, SoLoud::Soloud &soundEngine):
	m_assetLoader(assetLoader), m_inputManager(inputManager),
	m_soundEngine(soundEngine)
{
	// Initialize entity and component vectors.
	m_entities.resize(EntityConstants::MAX_ENTITIES);
	m_compPhysics.resize(EntityConstants::MAX_ENTITIES);
	m_compSprites.resize(EntityConstants::MAX_ENTITIES);
	m_compCollisions.resize(EntityConstants::MAX_ENTITIES);
	m_compWeapons.resize(EntityConstants::MAX_ENTITIES);
	m_compAttacks.resize(EntityConstants::MAX_ENTITIES);
	m_compEnemies.resize(EntityConstants::MAX_ENTITIES);
	m_compCharacters.resize(EntityConstants::MAX_ENTITIES);

	m_broadPhase = std::make_unique<CollisionBroadPhase>();

	// Initialize Lua state.
	initLua();

	m_effectsTexture = assetLoader->load<SpriteSheet>("effects");
	m_playerId = createEntity("serah", glm::vec2(64.f, 300.f));

	// Initialze game systems.
	m_gameSystems.emplace_back(std::make_unique<CharacterSystem>(*this,
		m_compSprites, m_compWeapons, m_compCollisions, m_compAttacks,
		m_compCharacters));
	m_gameSystems.emplace_back(std::make_unique<AttackSystem>(*this,
		m_compSprites, m_compAttacks));
	m_gameSystems.emplace_back(std::make_unique<AttackCollisionSystem>(*this,
		soundEngine, m_compPhysics, m_compSprites, m_compCollisions, 
		m_compAttacks, m_compCharacters, m_compPlayer));
	m_gameSystems.emplace_back(std::make_unique<PhysicsSystem>(*this,
		m_compPhysics, m_compCollisions, m_compCharacters));
	m_gameSystems.emplace_back(std::make_unique<SpriteSystem>(*this,
		sRenderer, m_compPhysics, m_compSprites, m_compWeapons));
	m_gameSystems.emplace_back(std::make_unique<RoomCollisionSystem>(*this,
		m_compPhysics, m_compCollisions, m_compCharacters));
	m_gameSystems.emplace_back(std::make_unique<PlayerSystem>(*this,
		m_compPlayer, m_compPhysics, m_compCollisions));
	m_gameSystems.emplace_back(std::make_unique<EnemySystem>(*this,
		m_compEnemies));

	m_debugSystem = std::make_unique<DebugSystem>(*this, uRenderer,
		m_compPhysics, m_compCollisions, m_compAttacks);
}

EntityManager::~EntityManager()
{

}

void EntityManager::update(float deltaTime, AssetLoader *assetLoader,
	UIRenderer *uRenderer, TextRenderer *tRenderer)
{
	m_deltaTime = deltaTime;

	// Get all collisions and handle them.
	m_broadPhase->updateAABBList(m_numEntities, m_entities, m_compCollisions,
		m_compAttacks, m_compPhysics);
	m_broadPhase->generateOverlapList(m_collisions);

	// Process all game systems.
	for (auto &system : m_gameSystems)
	{
		system->update(deltaTime, m_numEntities, m_entities);
	}

	// Reset overlaps list.
	m_collisions.clear();

	// Delete all flagged entities.
	deleteFlaggedEntities();

	// Update the hud with entity values.
	if (m_playerId != EntityConstants::PLAYER_NOT_SET)
	{
		const GameComponent::Character &playerChar{ m_compCharacters[m_playerId] };
		uRenderer->updateHud(assetLoader, tRenderer, m_compPlayer.portraitIcon,
			m_compPlayer.skillIcons, playerChar.health, playerChar.maxHealth,
			playerChar.resource, playerChar.maxResource, m_compPlayer.skillTimers);
	}
}

void EntityManager::updateDebug(float deltaTime)
{
	m_debugSystem->update(deltaTime, m_numEntities, m_entities);
}

int EntityManager::createEntity(std::vector<GameComponent::ComponentType> types)
{
	unsigned long mask = GameComponent::COMPONENT_NONE;
	for (auto &type : types)
	{
		GameComponent::addComponent(mask, type);
	}

	int id = m_numEntities;
	m_entities[id] = mask;
	m_numEntities++;

	return id;
}

int EntityManager::createEntity(std::string prefabName, glm::vec2 pos)
{
	std::shared_ptr<Prefab> prefab{ m_assetLoader->load<Prefab>(prefabName) };
	if (prefab == nullptr)
		return EntityConstants::PLAYER_NOT_SET;
	const json &json{ prefab->getJson() };

	unsigned long mask = GameComponent::COMPONENT_NONE;
	int id = m_numEntities;

	try
	{
		bool hasComponents{ JSONUtilities::hasEntry(PROPERTY_COMPONENTS, json) };
		if (hasComponents)
		{
			// Save the player/enemy's script name to execute at the end.
			std::string scriptName;

			bool hasCharacter{ false };
			bool hasPlayer{ false };
			bool hasAttack{ false };
			bool hasEnemy{ false };
			bool hasPhysics{ false };
			bool hasCollision{ false };

			auto &components{ json.at(PROPERTY_COMPONENTS) };
			if (components.is_array())
			{
				for (auto &component : components)
				{
					bool hasType{ JSONUtilities::hasEntry(PROPERTY_TYPE, component) };
					if (!hasType)
						continue;

					std::string type{ component.at(PROPERTY_TYPE).get<std::string>() };

					auto it{ LABEL_TO_COMPONENT.find(type) };
					if (it == LABEL_TO_COMPONENT.end())
						continue;

					GameComponent::addComponent(mask, it->second);

					if (type == COMPONENT_SPRITE)
					{
						hasPhysics = true;
					}
					if (type == COMPONENT_SPRITE)
						initializeSprite(id, component);
					else if (type == COMPONENT_PLAYER)
					{
						hasPlayer = true;
						scriptName = initializePlayer(component);
					}
					else if (type == COMPONENT_COLLISION)
					{
						hasCollision = true;
						initializeCollision(id, component);
					}
					else if (type == COMPONENT_WEAPON)
						initializeWeapon(id, component);
					else if (type == COMPONENT_ATTACK)
					{
						hasAttack = true;
						initializeAttack(id, component);
					}
					else if (type == COMPONENT_CHARACTER)
					{
						hasCharacter = true;
						initializeCharacter(id, component);
					}
					else if (type == COMPONENT_ENEMY)
					{
						hasEnemy = true;
						scriptName = initializeEnemy(id, component);
					}
				}
			}

			if (hasCharacter)
			{
				// Create the character's states.
				std::shared_ptr<Script> charStateScript{ m_assetLoader->load<Script>(CHAR_STATES_SCRIPT) };
				if (charStateScript != nullptr)
				{
					auto result{ charStateScript->execute(m_lua) };
					if (result.valid())
					{
						m_lua[INIT_CHAR_STATES](id);
					}
				}
			}

			// Set team id, if applicable.
			int team{ GameComponent::TEAM_NEUTRAL };
			if (hasPlayer)
			{
				team = GameComponent::TEAM_PLAYER;

				// Create the player's common states.
				std::shared_ptr<Script> playerStateScript{ m_assetLoader->load<Script>(PLAYER_STATES_SCRIPT) };
				if (playerStateScript != nullptr)
				{
					auto result{ playerStateScript->execute(m_lua) };
					if (result.valid())
					{
						m_lua[INIT_PLAYER_STATES](id);
					}
				}
			}
			else if (hasEnemy)
				team = GameComponent::TEAM_ENEMY;

			// Create the player's or enemy's specific states.
			if ((hasPlayer || hasEnemy) && !scriptName.empty())
			{
				std::shared_ptr<Script> statesScript{ m_assetLoader->load<Script>(scriptName) };
				if (statesScript != nullptr)
				{
					auto result{ statesScript->execute(m_lua) };
					if (result.valid())
					{
						m_lua[INIT_STATES](id);
					}
				}
			}

			if (hasCharacter)
				m_compCharacters[id].team = team;
			if (hasAttack)
				m_compAttacks[id].team = team;

			// Set initial position.
			if (hasPhysics && hasCollision)
			{
				GameComponent::Physics &phys{ m_compPhysics[id] };
				GameComponent::Collision &col{ m_compCollisions[id] };

				phys.pos.x = pos.x;
				phys.pos.y = pos.y - Room::TILE_SIZE / 2 + col.aabb.halfSize.y - col.aabb.offset.y;
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::createEntity: " << e.what() << std::endl;
	}

	m_entities[id] = mask;
	m_numEntities++;
	return id;
}

void EntityManager::initializeSprite(int entityId, const nlohmann::json &json)
{
	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	try
	{
		if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, json))
		{
			std::string spriteSheetLabel{ json.at(PROPERTY_SPRITESHEET).get<std::string>() };
			spr.spriteSheet = m_assetLoader->load<SpriteSheet>(spriteSheetLabel);
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializeSprite: " << e.what() << std::endl;
	}
}

std::string EntityManager::initializePlayer(const nlohmann::json &json)
{
	GameComponent::Player &player{ m_compPlayer };
	std::string scriptName;

	try
	{
		if (JSONUtilities::hasEntry(PROPERTY_EVADEDURATION, json))
		{
			player.evadeDuration = json.at(PROPERTY_EVADEDURATION).get<float>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_EVADESOUND, json))
		{
			std::string soundLabel{ json.at(PROPERTY_EVADESOUND).get<std::string>() };
			player.evadeSound = m_assetLoader->load<Sound>(soundLabel);
		}

		if (JSONUtilities::hasEntry(PROPERTY_PORTRAITICON, json))
		{
			player.portraitIcon = json.at(PROPERTY_PORTRAITICON).get<std::string>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_SKILLICONS, json))
		{
			player.skillIcons = json.at(PROPERTY_SKILLICONS).get<std::vector<std::string>>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_SCRIPT, json))
		{
			scriptName = json.at(PROPERTY_SCRIPT).get<std::string>();
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializePlayer: " << e.what() << std::endl;
	}

	return scriptName;
}

void EntityManager::initializeCollision(int entityId, const nlohmann::json &json)
{
	GameComponent::Collision &col{ m_compCollisions[entityId] };
	try
	{
		if (JSONUtilities::hasEntry(PROPERTY_BOXES, json))
		{
			nlohmann::json boxes{ json.at(PROPERTY_BOXES) };
			if (boxes.is_array())
			{
				// TODO: fix this to support multiple boxes.
				for (auto &box : boxes)
				{
					if (JSONUtilities::hasEntry(PROPERTY_HALFSIZE, box))
					{
						nlohmann::json sizeJson{ box.at(PROPERTY_HALFSIZE) };
						if (JSONUtilities::hasEntry(PROPERTY_X, sizeJson))
						{
							col.aabb.halfSize.x = sizeJson.at(PROPERTY_X).get<float>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_Y, sizeJson))
						{
							col.aabb.halfSize.y = sizeJson.at(PROPERTY_Y).get<float>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_OFFSET, box))
					{
						nlohmann::json offsetJson{ box.at(PROPERTY_OFFSET) };
						if (JSONUtilities::hasEntry(PROPERTY_X, offsetJson))
						{
							col.aabb.offset.x = offsetJson.at(PROPERTY_X).get<float>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_Y, offsetJson))
						{
							col.aabb.offset.y = offsetJson.at(PROPERTY_Y).get<float>();
						}
					}
				}
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializeCollision: " << e.what() << std::endl;
	}
}

void EntityManager::initializeWeapon(int entityId, const nlohmann::json &json)
{
	try
	{
		GameComponent::Weapon &wpn{ m_compWeapons[entityId] };
		if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, json))
		{
			std::string spriteSheetLabel{ json.at(PROPERTY_SPRITESHEET).get<std::string>() };
			wpn.spriteSheet = m_assetLoader->load<SpriteSheet>(spriteSheetLabel);
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializeWeapon: " << e.what() << std::endl;
	}
}

void EntityManager::initializeAttack(int entityId, const nlohmann::json &json)
{
	GameComponent::Attack &atk{ m_compAttacks[entityId] };
	atk.sourceId = entityId;
}

void EntityManager::initializeCharacter(int entityId, const nlohmann::json &json)
{
	GameComponent::Character &character{ m_compCharacters[entityId] };
	try
	{
		if (JSONUtilities::hasEntry(PROPERTY_HEALTH, json))
		{
			character.maxHealth = character.health = json.at(PROPERTY_HEALTH).get<int>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_RESOURCE, json))
		{
			character.maxResource = character.resource = json.at(PROPERTY_RESOURCE).get<int>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_MOVESPEED, json))
		{
			character.movementSpeed = character.movementSpeed = json.at(PROPERTY_MOVESPEED).get<float>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_JUMPSPEED, json))
		{
			character.jumpSpeed = character.jumpSpeed = json.at(PROPERTY_JUMPSPEED).get<float>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_ATTACKPATTERNS, json))
		{
			nlohmann::json patterns{ json.at(PROPERTY_ATTACKPATTERNS) };
			if (patterns.is_array())
			{
				for (auto &thisPattern : patterns)
				{
					AttackPattern atkPattern;

					// Can't uniquely identify this attack pattern without a "type",
					// so skip this.
					if (!JSONUtilities::hasEntry(PROPERTY_TYPE, thisPattern))
						continue;

					std::string type{ thisPattern.at(PROPERTY_TYPE).get<std::string>() };

					if (JSONUtilities::hasEntry(PROPERTY_HALFSIZE, thisPattern))
					{
						nlohmann::json sizeJson{ thisPattern.at(PROPERTY_HALFSIZE) };
						if (JSONUtilities::hasEntry(PROPERTY_X, sizeJson))
						{
							atkPattern.aabb.halfSize.x = sizeJson.at(PROPERTY_X).get<float>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_Y, sizeJson))
						{
							atkPattern.aabb.halfSize.y = sizeJson.at(PROPERTY_Y).get<float>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_OFFSET, thisPattern))
					{
						nlohmann::json offsetJson{ thisPattern.at(PROPERTY_OFFSET) };
						if (JSONUtilities::hasEntry(PROPERTY_X, offsetJson))
						{
							atkPattern.aabb.offset.x = offsetJson.at(PROPERTY_X).get<float>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_Y, offsetJson))
						{
							atkPattern.aabb.offset.y = offsetJson.at(PROPERTY_Y).get<float>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_HITENABLED, thisPattern))
					{
						nlohmann::json frameJson{ thisPattern.at(PROPERTY_HITENABLED) };
						if (JSONUtilities::hasEntry(PROPERTY_START, frameJson))
						{
							atkPattern.hitStart = frameJson.at(PROPERTY_START).get<int>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_NUMFRAMES, frameJson))
						{
							atkPattern.hitFrames = frameJson.at(PROPERTY_NUMFRAMES).get<int>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_SUPERARMOUR, thisPattern))
					{
						nlohmann::json frameJson{ thisPattern.at(PROPERTY_SUPERARMOUR) };
						if (JSONUtilities::hasEntry(PROPERTY_START, frameJson))
						{
							atkPattern.superArmourStart = frameJson.at(PROPERTY_START).get<int>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_NUMFRAMES, frameJson))
						{
							atkPattern.superArmourFrames = frameJson.at(PROPERTY_NUMFRAMES).get<int>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_COMBOFRAME, thisPattern))
					{
						atkPattern.comboFrame = thisPattern.at(PROPERTY_COMBOFRAME).get<int>();
					}

					if (JSONUtilities::hasEntry(PROPERTY_ATTACKSOUND, thisPattern))
					{
						std::string soundLabel{ thisPattern.at(PROPERTY_ATTACKSOUND).get<std::string>() };
						atkPattern.attackSound = m_assetLoader->load<Sound>(soundLabel);
					}

					if (JSONUtilities::hasEntry(PROPERTY_HITSOUND, thisPattern))
					{
						std::string soundLabel{ thisPattern.at(PROPERTY_HITSOUND).get<std::string>() };
						atkPattern.hitSound = m_assetLoader->load<Sound>(soundLabel);
					}

					if (JSONUtilities::hasEntry(PROPERTY_HITEFFECT, thisPattern))
					{
						atkPattern.hitSpark = thisPattern.at(PROPERTY_HITEFFECT).get<std::string>();
					}

					if (JSONUtilities::hasEntry(PROPERTY_COOLDOWN, thisPattern))
					{
						atkPattern.cooldown = thisPattern.at(PROPERTY_COOLDOWN).get<float>();
					}

					if (JSONUtilities::hasEntry(PROPERTY_DAMAGE, thisPattern))
					{
						atkPattern.damage = thisPattern.at(PROPERTY_DAMAGE).get<int>();
					}

					if (JSONUtilities::hasEntry(PROPERTY_KNOCKBACK, thisPattern))
					{
						nlohmann::json knockbackJson{ thisPattern.at(PROPERTY_KNOCKBACK) };
						if (JSONUtilities::hasEntry(PROPERTY_X, knockbackJson))
						{
							atkPattern.knockback.x = knockbackJson.at(PROPERTY_X).get<float>();
						}
						if (JSONUtilities::hasEntry(PROPERTY_Y, knockbackJson))
						{
							atkPattern.knockback.y = knockbackJson.at(PROPERTY_Y).get<float>();
						}
					}

					if (JSONUtilities::hasEntry(PROPERTY_HITSTUN, thisPattern))
					{
						atkPattern.hitStun = thisPattern.at(PROPERTY_HITSTUN).get<float>();
					}

					// Add this attack pattern.
					character.attackPatterns.insert({ type, atkPattern });
				}
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializeCharacter: " << e.what() << std::endl;
	}
}

std::string EntityManager::initializeEnemy(int entityId, const nlohmann::json &json)
{
	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	std::string scriptName;

	try
	{
		if (JSONUtilities::hasEntry(PROPERTY_TARGETRANGE, json))
		{
			nlohmann::json targetRangeJson{ json.at(PROPERTY_TARGETRANGE) };
			if (JSONUtilities::hasEntry(PROPERTY_X, targetRangeJson))
			{
				enemy.targetRange.x = targetRangeJson.at(PROPERTY_X).get<float>();
			}
			if (JSONUtilities::hasEntry(PROPERTY_Y, targetRangeJson))
			{
				enemy.targetRange.y = targetRangeJson.at(PROPERTY_Y).get<float>();
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_ATTACKRANGE, json))
		{
			nlohmann::json attackRangeJson{ json.at(PROPERTY_ATTACKRANGE) };
			if (JSONUtilities::hasEntry(PROPERTY_X, attackRangeJson))
			{
				enemy.attackRange.x = attackRangeJson.at(PROPERTY_X).get<float>();
			}
			if (JSONUtilities::hasEntry(PROPERTY_Y, attackRangeJson))
			{
				enemy.attackRange.y = attackRangeJson.at(PROPERTY_Y).get<float>();
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_ACTIONDURATION, json))
		{
			enemy.actionDuration = json.at(PROPERTY_ACTIONDURATION).get<float>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_ATTACKDURATION, json))
		{
			enemy.attackDuration = json.at(PROPERTY_ATTACKDURATION).get<float>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_SCRIPT, json))
		{
			scriptName = json.at(PROPERTY_SCRIPT).get<std::string>();
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "EntityManager::initializeEnemy: " << e.what() << std::endl;
	}

	return scriptName;
}

void EntityManager::deleteEntity(int id)
{
	// Flag the entity to be deleted at the end of the game loop.
	m_entitiesToDelete.push_back(id);
}


int EntityManager::getPlayerId() const
{
	return m_playerId;
}

glm::vec3 EntityManager::getPlayerPos() const
{
	if (m_playerId != EntityConstants::PLAYER_NOT_SET)
	{
		return m_compPhysics[m_playerId].pos;
	}

	return glm::vec3(0.f);
}

const std::vector<std::pair<AABBSource, AABBSource>> &EntityManager::getCollisions() const
{
	return m_collisions;
}

void EntityManager::createEffect(const std::string &type, glm::vec3 pos, 
	glm::vec2 scale, unsigned char r, unsigned char g, unsigned char b, 
	unsigned char a, float rotation)
{
	int effectId = createEntity(std::vector<GameComponent::ComponentType>{
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
	});

	GameComponent::Physics &phys{ m_compPhysics[effectId] };
	phys.pos = pos;
	phys.scale = scale;
	phys.rotation = rotation;

	GameComponent::Sprite &spr{ m_compSprites[effectId] };
	spr.r = r;
	spr.g = g;
	spr.b = b;
	spr.a = a;
	spr.isPersistent = false;
	spr.spriteSheet = m_effectsTexture;
	spr.spriteSheet->setSprite(type, spr);
}

int EntityManager::getNumEntities() const
{
	return m_numEntities;
}

void EntityManager::deleteFlaggedEntities()
{
	// Entities to delete should already be sorted in increasing order.
	for (int i = 0; i < m_entitiesToDelete.size();)
	{
		int id{ m_entitiesToDelete[i] };
		int idToDelete{ id };

		// Check if the last entity to delete is also the last entity (largest id).
		// If so, then this last entity should be deleted first, instead of
		// the current entity. This is to avoid swapping with the last entity,
		// which would lead to issues where the last entity is not deleted.
		int lastEntityId{ m_entitiesToDelete.back() };
		int lastIndex{ m_numEntities - 1 };
		if (lastEntityId == lastIndex)
		{
			idToDelete = lastEntityId;
		}

		// Delete the entity's components.
		m_compPhysics[idToDelete] = {};
		m_compSprites[idToDelete] = {};
		m_compCollisions[idToDelete] = {};
		m_compWeapons[idToDelete] = {};
		m_compAttacks[idToDelete] = {};
		m_compEnemies[idToDelete] = {};
		m_compCharacters[idToDelete] = {};


		// If the deleted entity was the player, reset the player component.
		if (idToDelete == m_playerId)
		{
			m_playerId = EntityConstants::PLAYER_NOT_SET;
			m_compPlayer = {};
		}

		m_entities[idToDelete] = GameComponent::COMPONENT_NONE;

		// If the current entity is being deleted, swap it with the last entity to keep the 
		// entities array tightly packed.
		bool isNotDeletingLast{ idToDelete != lastIndex};
		if (isNotDeletingLast)
		{
			unsigned long lastMask = m_entities[lastIndex];
			m_entities[id] = lastMask;
			m_entities[lastIndex] = GameComponent::COMPONENT_NONE;

			// Swap all components from the last entity.
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_PHYSICS))
			{
				std::iter_swap(m_compPhysics.begin() + id, m_compPhysics.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_SPRITE))
			{
				std::iter_swap(m_compSprites.begin() + id, m_compSprites.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_COLLISION))
			{
				std::iter_swap(m_compCollisions.begin() + id, m_compCollisions.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_WEAPON))
			{
				std::iter_swap(m_compWeapons.begin() + id, m_compWeapons.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_ATTACK))
			{
				std::iter_swap(m_compAttacks.begin() + id, m_compAttacks.begin() + lastIndex);

				// Update the attack's source.
				m_compAttacks[id].sourceId = id;
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_ENEMY))
			{
				std::iter_swap(m_compEnemies.begin() + id, m_compEnemies.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_CHARACTER))
			{
				std::iter_swap(m_compCharacters.begin() + id, m_compCharacters.begin() + lastIndex);
			}
			if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_PLAYER))
			{
				m_playerId = id;
			}

			// Deleting the current entity, so move onto the next id for the 
			// next iteration.
			++i;
		}
		else
		{
			// Deleting the last entity, so remove it from the list of
			// entities to be deleted. Continue to the next iteration with the same
			// current id.
			m_entitiesToDelete.pop_back();
		}

		m_numEntities--;
	}

	// Clear the list of flagged entities.
	m_entitiesToDelete.clear();
}

void EntityManager::initLua()
{
	m_lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, 
		sol::lib::os);

	// Initialize random seed.
	m_lua.script("math.randomseed(os.time())");

	m_lua.new_enum("InputType",
		"INPUT_UP", InputManager::INPUT_UP,
		"INPUT_DOWN", InputManager::INPUT_DOWN,
		"INPUT_LEFT", InputManager::INPUT_LEFT,
		"INPUT_RIGHT", InputManager::INPUT_RIGHT,
		"INPUT_CANCEL", InputManager::INPUT_CANCEL,
		"INPUT_ATTACK", InputManager::INPUT_ATTACK,
		"INPUT_JUMP", InputManager::INPUT_JUMP,
		"INPUT_EVADE", InputManager::INPUT_EVADE,
		"INPUT_SKILL1", InputManager::INPUT_SKILL1);

	m_lua.new_enum("CharState",
		"IDLE", CharState::IDLE,
		"RUN", CharState::RUN,
		"JUMP_ASCEND", CharState::JUMP_ASCEND,
		"JUMP_PEAK", CharState::JUMP_PEAK,
		"JUMP_DESCEND", CharState::JUMP_DESCEND,
		"JUMP_LAND", CharState::JUMP_LAND,
		"RUN_START", CharState::RUN_START,
		"RUN_STOP", CharState::RUN_STOP,
		"ALERT", CharState::ALERT,
		"ALERT_STOP", CharState::ALERT_STOP,
		"TURN", CharState::TURN,
		"CROUCH", CharState::CROUCH,
		"CROUCH_STOP", CharState::CROUCH_STOP,
		"ATTACK", CharState::ATTACK,
		"ATTACK_AIR", CharState::ATTACK_AIR,
		"ATTACK_CROUCH", CharState::ATTACK_CROUCH,
		"HURT", CharState::HURT,
		"HURT_AIR", CharState::HURT_AIR,
		"FALLEN", CharState::FALLEN,
		"EVADE_START", CharState::EVADE_START,
		"EVADE", CharState::EVADE,
		"ATTACK2", CharState::ATTACK2,
		"ATTACK3", CharState::ATTACK3,
		"SKILL1", CharState::SKILL1);

	m_lua.new_enum("EffectType",
		"EVADE_SMOKE", EffectType::EVADE_SMOKE,
		"HIT_SPARK", EffectType::HIT_SPARK);

	// Expose functions to Lua.
	m_lua.set_function("requireTable", [this](const std::string &scriptName)
		{
			std::shared_ptr<Script> script{ m_assetLoader->load<Script>(scriptName) };
			sol::table table;
			if (script != nullptr)
			{
				table = script->execute(m_lua);
			}

			return table;
		});
	m_lua.set_function("addState", [this](int entityId, const std::string &label) 
		{
			GameComponent::Character &character{ m_compCharacters[entityId] };
			StateMachine &states{ character.states };
			states.addState(label);
		});
	m_lua.set_function("addEdge", [this](int entityId, const std::string &srcLabel, 
		const std::string &destLabel, std::function<bool(int)> condition)
		{
			GameComponent::Character &character{ m_compCharacters[entityId] };
			StateMachine &states{ character.states };
			states.addEdge(srcLabel, destLabel, condition);
		});
	m_lua.set_function("setUpdateAction", [this](int entityId, const std::string &label,
		std::function<void(int)> action)
		{
			GameComponent::Character &character{ m_compCharacters[entityId] };
			StateMachine &states{ character.states };
			states.setUpdateAction(label, action);
		});
	m_lua.set_function("setEnterAction", [this](int entityId, const std::string &label,
		std::function<void(int)> action)
		{
			GameComponent::Character &character{ m_compCharacters[entityId] };
			StateMachine &states{ character.states };
			states.setEnterAction(label, action);
		});
	m_lua.set_function("setExitAction", [this](int entityId, const std::string &label,
		std::function<void(int)> action)
		{
			GameComponent::Character &character{ m_compCharacters[entityId] };
			StateMachine &states{ character.states };
			states.setExitAction(label, action);
		});
	m_lua.set_function("isKeyPressed", [this](InputManager::InputType input, bool flag)
		{
			return m_inputManager->isKeyPressed(input, flag);
		});
	m_lua.set_function("isKeyPressing", [this](InputManager::InputType input)
		{
			return m_inputManager->isKeyPressing(input);
		});
	m_lua.set_function("isKeyReleased", [this](InputManager::InputType input)
		{
			return m_inputManager->isKeyReleased(input);
		});
	m_lua.set_function("resetKeyDuration", [this](InputManager::InputType input)
		{
			return m_inputManager->resetDuration(input);
		});
	m_lua.set_function("playSound", [this](std::string soundName)
		{
			std::shared_ptr<Sound> sound{ m_assetLoader->load<Sound>(soundName) };
			sound->play(m_soundEngine, m_deltaTime);
		});
	m_lua.set_function("playAttackSound", [this](int entityId)
		{
			GameComponent::Attack &atk{ m_compAttacks[entityId] };
			if (atk.pattern.attackSound != nullptr)
				atk.pattern.attackSound->play(m_soundEngine, m_deltaTime);
		});
	m_lua.set_function("playEvadeSound", [this](int entityId)
		{
			if (m_compPlayer.evadeSound != nullptr)
				m_compPlayer.evadeSound->play(m_soundEngine, m_deltaTime);
		});
	m_lua.set_function("createEffect", [this](std::string type, 
		float posX, float posY, float scaleX, float scaleY, unsigned char r,
		unsigned char g, unsigned char b, unsigned char a, float rotation)
		{
			createEffect(type, glm::vec3(posX, posY, 0.f), 
				glm::vec2(scaleX, scaleY), r, g, b, a, rotation);
		});
	m_lua.set_function("getDeltaTime", [this]()
		{
			return m_deltaTime;
		});
	m_lua.set_function("clamp", [this](float value, float min, float max)
		{
			return glm::clamp(value, min, max);
		});
	m_lua.set_function("sign", [this](float value)
		{
			return glm::sign(value);
		});
	m_lua.set_function("getPosX", [this](int entityId)
		{
			return m_compPhysics[entityId].pos.x;
		});
	m_lua.set_function("getPosY", [this](int entityId)
		{
			return m_compPhysics[entityId].pos.y;
		});
	m_lua.set_function("getSpeedX", [this](int entityId) 
		{ 
			return m_compPhysics[entityId].speed.x; 
		});
	m_lua.set_function("getSpeedY", [this](int entityId) 
		{ 
			return m_compPhysics[entityId].speed.y; 
		});
	m_lua.set_function("getScaleX", [this](int entityId)
		{
			return m_compPhysics[entityId].scale.x;
		});
	m_lua.set_function("getScaleY", [this](int entityId)
		{
			return m_compPhysics[entityId].scale.y;
		});
	m_lua.set_function("hasFriction", [this](int entityId)
		{
			return m_compPhysics[entityId].hasFriction;
		});
	m_lua.set_function("hasGravity", [this](int entityId)
		{
			return m_compPhysics[entityId].hasGravity;
		});
	m_lua.set_function("isInAir", [this](int entityId) 
		{ 
			return GameComponent::isInAir(m_compPhysics[entityId], 
				m_compCollisions[entityId]); 
		});
	m_lua.set_function("isOnGround", [this](int entityId)
		{
			return GameComponent::isOnGround(m_compPhysics[entityId],
				m_compCollisions[entityId]);
		});
	m_lua.set_function("isLockedDirection", [this](int entityId)
		{
			return m_compPhysics[entityId].isLockedDirection;
		});
	m_lua.set_function("getPlayerPosX", [this]()
		{
			if (m_playerId != EntityConstants::PLAYER_NOT_SET)
				return m_compPhysics[m_playerId].pos.x;
			else
				return -1.f;
		});
	m_lua.set_function("getPlayerPosY", [this]()
		{
			if (m_playerId != EntityConstants::PLAYER_NOT_SET)
				return m_compPhysics[m_playerId].pos.y;
			else
				return -1.f;
		});
	m_lua.set_function("setPosX", [this](int entityId, float value)
		{
			m_compPhysics[entityId].pos.x = value;
		});
	m_lua.set_function("setPosY", [this](int entityId, float value)
		{
			m_compPhysics[entityId].pos.y = value;
		});
	m_lua.set_function("setSpeedX", [this](int entityId, float value)
		{
			m_compPhysics[entityId].speed.x = value;
		});
	m_lua.set_function("setSpeedY", [this](int entityId, float value)
		{
			m_compPhysics[entityId].speed.y = value;
		});
	m_lua.set_function("setScaleX", [this](int entityId, float value)
		{
			m_compPhysics[entityId].scale.x = value;
		});
	m_lua.set_function("setScaleY", [this](int entityId, float value)
		{
			m_compPhysics[entityId].scale.y = value;
		});
	m_lua.set_function("setFriction", [this](int entityId, bool flag)
		{
			m_compPhysics[entityId].hasFriction = flag;
		});
	m_lua.set_function("setGravity", [this](int entityId, bool flag)
		{
			m_compPhysics[entityId].hasGravity = flag;
		});
	m_lua.set_function("setLockedDirection", [this](int entityId, bool flag)
		{
			m_compPhysics[entityId].isLockedDirection = flag;
		});
	m_lua.set_function("isCollidingHorizontal", [this](int entityId)
		{
			return m_compCollisions[entityId].isCollidingHorizontal;
		});
	m_lua.set_function("isCollidingFloor", [this](int entityId)
		{
			return m_compCollisions[entityId].isCollidingFloor;
		});
	m_lua.set_function("isCollidingGhost", [this](int entityId)
		{
			return m_compCollisions[entityId].isCollidingGhost;
		});
	m_lua.set_function("isCollidingSlope", [this](int entityId)
		{
			return m_compCollisions[entityId].isCollidingSlope;
		});
	m_lua.set_function("getState", [this](int entityId)
		{
			return m_compCharacters[entityId].states.getState();
		});
	m_lua.set_function("getPreviousState", [this](int entityId)
		{
			return m_compCharacters[entityId].previousState;
		});
	m_lua.set_function("getMovementSpeed", [this](int entityId)
		{
			return m_compCharacters[entityId].movementSpeed;
		});
	m_lua.set_function("getJumpSpeed", [this](int entityId)
		{
			return m_compCharacters[entityId].jumpSpeed;
		});
	m_lua.set_function("getHitStunTimer", [this](int entityId)
		{
			return m_compCharacters[entityId].hitStunTimer;
		});
	m_lua.set_function("getFallenTimer", [this](int entityId)
		{
			return m_compCharacters[entityId].fallenTimer;
		});
	m_lua.set_function("getMaxHealth", [this](int entityId)
		{
			return m_compCharacters[entityId].maxHealth;
		});
	m_lua.set_function("isDead", [this](int entityId)
		{
			return GameComponent::isDead(m_compCharacters[entityId]);
		});
	m_lua.set_function("setFallenTimer", [this](int entityId, float value)
		{
			m_compCharacters[entityId].fallenTimer = value;
		});
	m_lua.set_function("setInvincibilityTimer", [this](int entityId, float value)
		{
			m_compCharacters[entityId].invincibilityTimer = value;
		});
	m_lua.set_function("getNumRemainingJumps", [this](int entityId)
		{
			return m_compPlayer.numRemainingJumps;
		});
	m_lua.set_function("getNumRemainingEvades", [this](int entityId)
		{
			return m_compPlayer.numRemainingEvades;
		});
	m_lua.set_function("getMaxEvades", [this](int entityId)
		{
			return m_compPlayer.numMaxEvades;
		});
	m_lua.set_function("getEvadeTimer", [this](int entityId)
		{
			return m_compPlayer.evadeTimer;
		});
	m_lua.set_function("getSkillTimer", [this](int entityId, int index)
		{
			return m_compPlayer.skillTimers[index];
		});
	m_lua.set_function("getRecentHealthLost", [this](int entityId)
		{
			return m_compPlayer.recentHealthLost;
		});
	m_lua.set_function("setNumRemainingJumps", [this](int entityId, int value)
		{
			m_compPlayer.numRemainingJumps = value;
		});
	m_lua.set_function("setNumRemainingEvades", [this](int entityId, int value)
		{
			m_compPlayer.numRemainingEvades = value;
		});
	m_lua.set_function("setEvadeTimer", [this](int entityId)
		{
			m_compPlayer.evadeTimer = m_compPlayer.evadeDuration;
		});
	m_lua.set_function("setSkillTimer", [this](int entityId, int index)
		{
			m_compPlayer.skillTimers[index] = m_compAttacks[entityId].pattern.cooldown;
		});
	m_lua.set_function("resetRecentlyHit", [this](int entityId)
		{
			m_compPlayer.recentlyHitTimer = 0.f;
			m_compPlayer.recentHealthLost = 0;
		});
	m_lua.set_function("getCurrentFrameTime", [this](int entityId)
		{
			return m_compSprites[entityId].currentFrameTime;
		});
	m_lua.set_function("getFrameDuration", [this](int entityId)
		{
			return GameComponent::getFrameDuration(m_compSprites[entityId]);
		});
	m_lua.set_function("isSpriteLooping", [this](int entityId)
		{
			return m_compSprites[entityId].currentSprite.isLooping;
		});
	m_lua.set_function("getCurrentFrame", [this](int entityId)
		{
			return m_compSprites[entityId].currentFrame;
		}); 
	m_lua.set_function("getNumSprites", [this](int entityId)
		{
			return GameComponent::getNumSprites(m_compSprites[entityId]);
		});
	m_lua.set_function("setResetAnimation", [this](int entityId, bool flag)
		{
			m_compSprites[entityId].isResetAnimation = flag;
		});
	m_lua.set_function("setCurrentFrame", [this](int entityId, int frame)
		{
			m_compSprites[entityId].currentFrame = frame;
		});
	m_lua.set_function("setCurrentFrameTime", [this](int entityId, float value)
		{
			m_compSprites[entityId].currentFrameTime = value;
		});
	m_lua.set_function("setSpriteAlpha", [this](int entityId, unsigned char a)
		{
			m_compSprites[entityId].a = a;
		});
	m_lua.set_function("getAttackFrameStart", [this](int entityId)
		{
			return m_compAttacks[entityId].pattern.hitStart;
		});
	m_lua.set_function("getAttackKnockbackY", [this](int entityId)
		{
			return m_compAttacks[entityId].pattern.knockback.y;
		});
	m_lua.set_function("getComboFrame", [this](int entityId)
		{
			return m_compAttacks[entityId].pattern.comboFrame;
		});
	m_lua.set_function("isEnemyMovingLeft", [this](int entityId)
		{
			return m_compEnemies[entityId].isMovingLeft;
		});
	m_lua.set_function("isEnemyMovingRight", [this](int entityId)
		{
			return m_compEnemies[entityId].isMovingRight;
		});
	m_lua.set_function("isEnemyTargetingPlayer", [this](int entityId)
		{
			return m_compEnemies[entityId].isTargetingPlayer;
		});
	m_lua.set_function("getEnemyTargetRangeX", [this](int entityId)
		{
			return m_compEnemies[entityId].targetRange.x;
		});
	m_lua.set_function("getEnemyTargetRangeY", [this](int entityId)
		{
			return m_compEnemies[entityId].targetRange.y;
		});
	m_lua.set_function("getEnemyAttackRangeX", [this](int entityId)
		{
			return m_compEnemies[entityId].attackRange.x;
		});
	m_lua.set_function("getEnemyAttackRangeY", [this](int entityId)
		{
			return m_compEnemies[entityId].attackRange.y;
		});
	m_lua.set_function("getEnemyActionTimer", [this](int entityId)
		{
			return m_compEnemies[entityId].actionTimer;
		});
	m_lua.set_function("getEnemyAttackTimer", [this](int entityId)
		{
			return m_compEnemies[entityId].attackTimer;
		});
	m_lua.set_function("setEnemyMovingLeft", [this](int entityId, bool flag)
		{
			m_compEnemies[entityId].isMovingLeft = flag;

			if (flag)
				m_compEnemies[entityId].isMovingRight = false;
		});
	m_lua.set_function("setEnemyMovingRight", [this](int entityId, bool flag)
		{
			m_compEnemies[entityId].isMovingRight = flag;

			if (flag)
				m_compEnemies[entityId].isMovingLeft = false;
		});
	m_lua.set_function("setEnemyTargetingPlayer", [this](int entityId, bool flag)
		{
			return m_compEnemies[entityId].isTargetingPlayer = flag;
		});
	m_lua.set_function("setEnemyActionTimer", [this](int entityId, float value)
		{
			GameComponent::setActionTimer(m_compEnemies[entityId], value);
		});
	m_lua.set_function("setEnemyAttackTimer", [this](int entityId)
		{
			return m_compEnemies[entityId].attackTimer = m_compEnemies[entityId].attackDuration;
		});
}