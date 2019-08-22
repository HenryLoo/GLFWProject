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
	const std::string PROPERTY_FRAMERANGE{ "frameRange" };
	const std::string PROPERTY_START{ "start" };
	const std::string PROPERTY_END{ "end" };
	const std::string PROPERTY_COMBOFRAME{ "comboFrame" };
	const std::string PROPERTY_ATTACKSOUND{ "attackSound" };
	const std::string PROPERTY_HITSOUND{ "hitSound" };
	const std::string PROPERTY_HITEFFECT{ "hitEffect" };
	const std::string PROPERTY_COOLDOWN{ "cooldown" };
	const std::string PROPERTY_DAMAGE{ "damage" };
	const std::string PROPERTY_KNOCKBACK{ "knockback" };

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

	// TODO: replace these hardcoded resources.
	m_effectsTexture = assetLoader->load<SpriteSheet>("effects");
	createEnemy();
	createPlayer();

	// Initialze game systems.
	m_gameSystems.emplace_back(std::make_unique<AttackSystem>(*this,
		m_compSprites, m_compAttacks));
	m_gameSystems.emplace_back(std::make_unique<AttackCollisionSystem>(*this,
		soundEngine, m_compPhysics, m_compSprites, m_compCollisions, 
		m_compAttacks, m_compCharacters));
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
	m_gameSystems.emplace_back(std::make_unique<CharacterSystem>(*this,
		m_compSprites, m_compWeapons, m_compCollisions, m_compAttacks,
		m_compCharacters));

	m_debugSystem = std::make_unique<DebugSystem>(*this, uRenderer,
		m_compPhysics, m_compCollisions, m_compAttacks);

	// TODO: remove this later.
	//auto testScript{ assetLoader->load<Script>("test") };
	//if (testScript->execute(m_lua))
	//{
	//	m_lua.set("testMsg", "TEST MSG!");
	//	m_lua["testFun"]();
	//}
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

int EntityManager::createEntity(Prefab *prefab)
{
	const json &json{ prefab->getJson() };

	unsigned long mask = GameComponent::COMPONENT_NONE;
	int id = m_numEntities;

	bool hasComponents{ JSONUtilities::hasEntry(PROPERTY_COMPONENTS, json) };
	if (hasComponents)
	{
		// Save the player/enemy's script name to execute at the end.
		std::string scriptName;

		bool hasCharacter{ false };
		bool hasPlayer{ false };
		bool hasAttack{ false };
		bool hasEnemy{ false };

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
					initializeSprite(id, component);
				else if (type == COMPONENT_PLAYER)
				{
					hasPlayer = true;
					scriptName = initializePlayer(component);
				}
				else if (type == COMPONENT_COLLISION)
					initializeCollision(id, component);
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
	}

	m_entities[id] = mask;
	m_numEntities++;
	return id;
}

void EntityManager::initializeSprite(int entityId, const nlohmann::json &json)
{
	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, json))
	{
		std::string spriteSheetLabel{ json.at(PROPERTY_SPRITESHEET).get<std::string>() };
		spr.spriteSheet = m_assetLoader->load<SpriteSheet>(spriteSheetLabel);
	}
}

std::string EntityManager::initializePlayer(const nlohmann::json &json)
{
	GameComponent::Player &player{ m_compPlayer };
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

	std::string scriptName;
	if (JSONUtilities::hasEntry(PROPERTY_SCRIPT, json))
	{
		scriptName = json.at(PROPERTY_SCRIPT).get<std::string>();
	}

	return scriptName;
}

void EntityManager::initializeCollision(int entityId, const nlohmann::json &json)
{
	GameComponent::Collision &col{ m_compCollisions[entityId] };
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

void EntityManager::initializeWeapon(int entityId, const nlohmann::json &json)
{
	GameComponent::Weapon &wpn{ m_compWeapons[entityId] };
	if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, json))
	{
		std::string spriteSheetLabel{ json.at(PROPERTY_SPRITESHEET).get<std::string>() };
		wpn.spriteSheet = m_assetLoader->load<SpriteSheet>(spriteSheetLabel);
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

				if (JSONUtilities::hasEntry(PROPERTY_FRAMERANGE, thisPattern))
				{
					nlohmann::json frameJson{ thisPattern.at(PROPERTY_FRAMERANGE) };
					if (JSONUtilities::hasEntry(PROPERTY_START, frameJson))
					{
						atkPattern.frameRange.x = frameJson.at(PROPERTY_START).get<int>();
					}
					if (JSONUtilities::hasEntry(PROPERTY_END, frameJson))
					{
						atkPattern.frameRange.y = frameJson.at(PROPERTY_END).get<int>();
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

				// Add this attack pattern.
				character.attackPatterns.insert({ type, atkPattern });
			}
		}
	}
}

std::string EntityManager::initializeEnemy(int entityId, const nlohmann::json &json)
{
	std::string scriptName;
	if (JSONUtilities::hasEntry(PROPERTY_SCRIPT, json))
	{
		scriptName = json.at(PROPERTY_SCRIPT).get<std::string>();
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
	int effectId = createEntity({
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
	for (int id : m_entitiesToDelete)
	{
		// Delete the entity's components.
		m_compPhysics[id] = {};
		m_compSprites[id] = {};
		m_compCollisions[id] = {};
		m_compWeapons[id] = {};
		m_compAttacks[id] = {};
		m_compEnemies[id] = {};
		m_compCharacters[id] = {};

		// Swap with the last entity to keep the entities array tightly packed.
		int lastIndex = m_numEntities - 1;
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

		m_numEntities--;

		// If the last entity was the player, update the player id.
		if (m_playerId == lastIndex)
			m_playerId = id;
		// If the deleted entity was the player, reset the player component.
		else if (m_playerId == id)
		{
			m_playerId = -1;
			m_compPlayer = {};
		}
	}

	// Clear the list of flagged entities.
	m_entitiesToDelete.clear();
}

void EntityManager::createPlayer()
{
	std::shared_ptr<Prefab> playerPrefab{ m_assetLoader->load<Prefab>("serah") };
	m_playerId = createEntity(playerPrefab.get());

	GameComponent::Physics &phys = m_compPhysics[m_playerId];
	phys.pos = glm::vec3(64.f, 800.f, 0.f);
	//GameComponent::Sprite &spr = m_compSprites[m_playerId];
	//GameComponent::Weapon &wp = m_compWeapons[m_playerId];
	//GameComponent::Collision &col = m_compCollisions[m_playerId];
	//GameComponent::Attack &atk = m_compAttacks[m_playerId];
	//GameComponent::Character &character = m_compCharacters[m_playerId];

	//// Set up state machine.
	//StateMachine &states{ character.states };

	//auto enableFriction{ [this](int entityId)
	//{
	//	m_compPhysics[entityId].hasFriction = true;
	//} };

	//auto enableGravity{ [this](int entityId)
	//{
	//	m_compPhysics[entityId].hasGravity = true;
	//} };

	//auto evadeUpdateAction{ [this](int entityId)
	//{
	//	// If in the air, reset vertical speed and disable gravity.
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	if (GameComponent::isInAir(phys, col))
	//	{
	//		phys.speed.y = 0.f;
	//		phys.hasGravity = false;
	//	}
	//	else
	//	{
	//		phys.hasGravity = true;
	//	}
	//} };

	//auto evadeEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	// By default, just evade in the direction that
	//	// the player is facing.
	//	float dir{ glm::sign(phys.scale.x) };

	//	// If left/right is held down, then evade in that direction
	//	// instead.
	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };
	//	if (isRunning)
	//	{
	//		if (isRunningLeft)
	//			dir = -1.f;
	//		else if (isRunningRight)
	//			dir = 1.f;
	//	}

	//	m_compPlayer.numRemainingEvades--;
	//	phys.speed.x = dir * 1.7f * character.movementSpeed;
	//	phys.scale.x = glm::abs(phys.scale.x) * dir;
	//	phys.hasFriction = false;
	//	phys.isLockedDirection = false;

	//	// Set the evade timer.
	//	m_compPlayer.evadeTimer = m_compPlayer.evadeDuration;

	//	// Create smoke effect.
	//	glm::vec3 effectPos{ phys.pos };
	//	effectPos.x -= (dir * 20.f);
	//	effectPos.y -= 4.f;
	//	createEffect(EffectType::EVADE_SMOKE, effectPos, phys.scale, 
	//		255, 255, 255, 180);

	//	m_evadeSound->play(m_soundEngine);
	//} };

	//auto runUpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };

	//	// Stopping while turning.
	//	bool isStopRunningLeft{ m_inputManager->isKeyReleased(InputManager::INPUT_LEFT) };
	//	bool isStopRunningRight{ m_inputManager->isKeyReleased(InputManager::INPUT_RIGHT) };
	//	bool isTurning{ character.states.getState() == CharState::TURN };
	//	if (isTurning && ((isStopRunningLeft && isStopRunningRight) || !isRunning))
	//	{
	//		phys.hasFriction = true;
	//		return;
	//	}

	//	// If current speed is greater than the character's movement speed,
	//	// start applying friction to gradually bring it back down to
	//	// the character's movement speed.
	//	float currentSpeed{ glm::abs(phys.speed.x) };
	//	phys.hasFriction = (currentSpeed > character.movementSpeed);

	//	// Maintain maximum horizontal speed.
	//	float maxSpeed{ glm::max(character.movementSpeed, currentSpeed) };

	//	float dir{ 0.f };
	//	if (isRunning)
	//	{
	//		if (isRunningLeft)
	//			dir = -1.f;
	//		else if (isRunningRight)
	//			dir = 1.f;
	//	}

	//	phys.speed.x += (dir * character.movementSpeed / 0.1f * m_deltaTime);
	//	phys.speed.x = glm::clamp(phys.speed.x, -maxSpeed, maxSpeed);

	//	if (dir != 0.f && !phys.isLockedDirection)
	//		phys.scale.x = glm::abs(phys.scale.x) * dir;
	//} };

	//auto dropDownEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };

	//	bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
	//	bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP) };
	//	if (isCrouching && isJumping && col.isCollidingGhost &&
	//		!col.isCollidingFloor && !col.isCollidingSlope)
	//	{
	//		spr.isResetAnimation = true;
	//		m_compPlayer.numRemainingJumps--;
	//		phys.speed.y = 0.f;
	//		phys.pos.y--;
	//	}
	//} };

	//auto attackAirEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	phys.isLockedDirection = true;
	//} };

	//auto jumpLandEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	phys.isLockedDirection = false;
	//	phys.hasFriction = true;
	//} };

	//auto turnEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	spr.isResetAnimation = true;
	//} };

	//auto skill1EnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Attack &atk{ m_compAttacks[entityId] };

	//	m_compPlayer.skillTimers[0] = atk.pattern.cooldown;
	//	phys.speed.x = 0.f;
	//	if (GameComponent::isInAir(phys, col))
	//		phys.speed.y = 0.f;
	//	phys.hasGravity = false;
	//} };

	//auto skill1ExitAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	phys.hasGravity = true;
	//} };

	//auto skill1UpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Attack &atk{ m_compAttacks[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };

	//	if (spr.currentFrame == atk.pattern.frameRange.x && !phys.hasGravity)
	//	{
	//		float dir{ glm::sign(phys.scale.x) };
	//		phys.speed.x = dir * 16.f;
	//		phys.speed.y = atk.pattern.knockback.y - 16.f;
	//		phys.isLockedDirection = true;
	//		phys.hasGravity = true;
	//	}
	//} };

	//auto fallenEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	phys.hasFriction = true;
	//	character.fallenTimer = 3.f;
	//} };

	//auto fallenUpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Character &character{ m_compCharacters[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };

	//	// If dead, make sprite translucent.
	//	if (GameComponent::isDead(character))
	//	{
	//		spr.a = 100;
	//	}
	//} };

	//states.addState(CharState::IDLE);
	//states.addState(CharState::RUN, runUpdateAction, [](int) {}, enableFriction);
	//states.addState(CharState::JUMP_ASCEND, runUpdateAction);
	//states.addState(CharState::JUMP_PEAK, runUpdateAction, dropDownEnterAction);
	//states.addState(CharState::JUMP_DESCEND, runUpdateAction);
	//states.addState(CharState::JUMP_LAND, [](int) {}, jumpLandEnterAction);
	//states.addState(CharState::RUN_START, runUpdateAction, [](int) {}, enableFriction);
	//states.addState(CharState::RUN_STOP);
	//states.addState(CharState::ALERT);
	//states.addState(CharState::ALERT_STOP);
	//states.addState(CharState::TURN, runUpdateAction, turnEnterAction, enableFriction);
	//states.addState(CharState::CROUCH, [](int) {}, enableFriction);
	//states.addState(CharState::CROUCH_STOP);
	//states.addState(CharState::ATTACK, [](int) {}, enableFriction);
	//states.addState(CharState::ATTACK2, [](int) {}, enableFriction);
	//states.addState(CharState::ATTACK3, [](int) {}, enableFriction);
	//states.addState(CharState::ATTACK_AIR, runUpdateAction, attackAirEnterAction);
	//states.addState(CharState::ATTACK_CROUCH);
	//states.addState(CharState::EVADE_START, evadeUpdateAction, evadeEnterAction, enableGravity);
	//states.addState(CharState::EVADE, evadeUpdateAction, [](int) {}, enableGravity);
	//states.addState(CharState::SKILL1, skill1UpdateAction, skill1EnterAction, skill1ExitAction);
	//states.addState(CharState::HURT);
	//states.addState(CharState::HURT_AIR);
	//states.addState(CharState::FALLEN, fallenUpdateAction, fallenEnterAction);

	//// Hurt while on ground.
	//auto isHurting{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (character.hitStunTimer > 0.f && GameComponent::isOnGround(phys, col));
	//} };
	//states.addEdge(CharState::IDLE, CharState::HURT, isHurting);
	//states.addEdge(CharState::ALERT, CharState::HURT, isHurting);
	//states.addEdge(CharState::ALERT_STOP, CharState::HURT, isHurting);
	//states.addEdge(CharState::RUN_START, CharState::HURT, isHurting);
	//states.addEdge(CharState::RUN, CharState::HURT, isHurting);
	//states.addEdge(CharState::RUN_STOP, CharState::HURT, isHurting);
	//states.addEdge(CharState::TURN, CharState::HURT, isHurting);
	//states.addEdge(CharState::CROUCH_STOP, CharState::HURT, isHurting);
	//states.addEdge(CharState::JUMP_LAND, CharState::HURT, isHurting);
	//states.addEdge(CharState::ATTACK, CharState::HURT, isHurting);
	//states.addEdge(CharState::ATTACK2, CharState::HURT, isHurting);
	//states.addEdge(CharState::ATTACK3, CharState::HURT, isHurting);
	//states.addEdge(CharState::ATTACK_CROUCH, CharState::HURT, isHurting);
	//states.addEdge(CharState::EVADE_START, CharState::HURT, isHurting);
	//states.addEdge(CharState::EVADE, CharState::HURT, isHurting);
	//states.addEdge(CharState::SKILL1, CharState::HURT, isHurting);

	//// Stop hurting.
	//auto isStopHurting{ [this](int entityId) -> bool
	//{
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return character.hitStunTimer == 0.f;
	//} };
	//states.addEdge(CharState::HURT, CharState::IDLE, isStopHurting);

	//// Hurt while in the air.
	//auto isHurtingAir{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (character.hitStunTimer > 0.f && GameComponent::isInAir(phys, col));
	//} };
	//states.addEdge(CharState::HURT, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::FALLEN, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::JUMP_PEAK, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::ATTACK_AIR, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::EVADE_START, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::EVADE, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::SKILL1, CharState::HURT_AIR, isHurtingAir);

	//// Fallen.
	//auto isFallen{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	return GameComponent::isOnGround(phys, col);
	//} };
	//states.addEdge(CharState::HURT_AIR, CharState::FALLEN, isFallen);

	//// Dead while on ground.
	//auto isDead{ [this](int entityId) -> bool
	//{
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return GameComponent::isDead(character);
	//} };
	//states.addEdge(CharState::HURT, CharState::FALLEN, isDead);

	//// Stop fallen.
	//auto isStopFallen{ [this](int entityId) -> bool
	//{
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return character.fallenTimer == 0.f && !GameComponent::isDead(character);
	//} };
	//states.addEdge(CharState::FALLEN, CharState::CROUCH_STOP, isStopFallen);

	//// Animation end transitions.
	//auto isAnimationEnd{ [this](int entityId) -> bool
	//{
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };

	//	float frameDuration{ GameComponent::getFrameDuration(spr) };
	//	int numSprites{ GameComponent::getNumSprites(spr) };
	//	return (spr.currentFrameTime >= frameDuration &&
	//		(!spr.currentSprite.isLooping && spr.currentFrame == numSprites - 1));
	//} };
	//states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_DESCEND, isAnimationEnd);
	//states.addEdge(CharState::JUMP_LAND, CharState::CROUCH_STOP, isAnimationEnd);
	//states.addEdge(CharState::CROUCH_STOP, CharState::IDLE, isAnimationEnd);
	//states.addEdge(CharState::RUN_START, CharState::RUN, isAnimationEnd);
	//states.addEdge(CharState::RUN_STOP, CharState::ALERT, isAnimationEnd);
	//states.addEdge(CharState::TURN, CharState::RUN_START, isAnimationEnd);
	//states.addEdge(CharState::ALERT, CharState::ALERT_STOP, isAnimationEnd);
	//states.addEdge(CharState::ALERT_STOP, CharState::IDLE, isAnimationEnd);
	//states.addEdge(CharState::ATTACK, CharState::ALERT_STOP, isAnimationEnd);
	//states.addEdge(CharState::ATTACK2, CharState::ALERT_STOP, isAnimationEnd);
	//states.addEdge(CharState::ATTACK3, CharState::ALERT_STOP, isAnimationEnd);
	//states.addEdge(CharState::ATTACK_CROUCH, CharState::CROUCH, isAnimationEnd);
	//states.addEdge(CharState::EVADE_START, CharState::EVADE, isAnimationEnd);
	//states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_ASCEND, isAnimationEnd);

	//// Falling.
	//auto isFalling{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (phys.speed.y < 0 && GameComponent::isInAir(phys, col) &&
	//		character.states.getState() != CharState::JUMP_DESCEND);
	//} };
	//states.addEdge(CharState::IDLE, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::RUN_START, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::RUN, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::RUN_STOP, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::TURN, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::CROUCH_STOP, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::ATTACK, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::ATTACK_CROUCH, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::ATTACK2, CharState::JUMP_PEAK, isFalling);
	//states.addEdge(CharState::ATTACK3, CharState::JUMP_PEAK, isFalling);

	//// Landing.
	//auto isLanding{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	return GameComponent::isOnGround(phys, col);
	//} };
	//states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_LAND, isLanding);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_LAND, isLanding);
	//states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_LAND, isLanding);

	//// Skill 1.
	//auto isSkill1{ [this](int entityId) -> bool
	//{
	//	bool isSkill{ m_inputManager->isKeyPressed(InputManager::INPUT_SKILL1) &&
	//		m_compPlayer.skillTimers[0] == 0.f };

	//	if (isSkill)
	//	{
	//		m_attackSound->play(m_soundEngine);
	//	}
	//	return isSkill;
	//} };
	//states.addEdge(CharState::IDLE, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ALERT, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ALERT_STOP, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::RUN_START, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::RUN, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::RUN_STOP, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::TURN, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::CROUCH_STOP, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::JUMP_LAND, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::JUMP_PEAK, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ATTACK, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ATTACK2, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ATTACK3, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ATTACK_AIR, CharState::SKILL1, isSkill1);
	//states.addEdge(CharState::ATTACK_CROUCH, CharState::SKILL1, isSkill1);

	//auto isSkill1End{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Attack &atk{ m_compAttacks[entityId] };

	//	return (phys.speed.y < 0.f &&
	//		spr.currentFrame > atk.pattern.frameRange.x);
	//} };
	//states.addEdge(CharState::SKILL1, CharState::JUMP_ASCEND, isSkill1End);

	//// Attacking.
	//auto isAttacking{ [this](int entityId) -> bool
	//{
	//	bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) };
	//	if (isAttacking)
	//	{
	//		m_attackSound->play(m_soundEngine);
	//	}
	//	return isAttacking;
	//} };
	//states.addEdge(CharState::IDLE, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::ALERT, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::ALERT_STOP, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::RUN_START, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::RUN, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::RUN_STOP, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::TURN, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::CROUCH, CharState::ATTACK_CROUCH, isAttacking);
	//states.addEdge(CharState::CROUCH_STOP, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::JUMP_LAND, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::ATTACK_AIR, isAttacking);
	//states.addEdge(CharState::JUMP_PEAK, CharState::ATTACK_AIR, isAttacking);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::ATTACK_AIR, isAttacking);
	//states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking);

	//auto isAttackCombo{ [this](int entityId) -> bool
	//{
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Attack &atk{ m_compAttacks[entityId] };

	//	bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) && 
	//		spr.currentFrame > atk.pattern.comboFrame };
	//	if (isAttacking)
	//	{
	//		m_attackSound->play(m_soundEngine);
	//	}
	//	return isAttacking;
	//} };
	//states.addEdge(CharState::ATTACK, CharState::ATTACK2, isAttackCombo);
	//states.addEdge(CharState::ATTACK2, CharState::ATTACK3, isAttackCombo);

	//auto isAirAttacking{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) };
	//	return (isAttacking && GameComponent::isInAir(phys, col));
	//} };
	//states.addEdge(CharState::EVADE, CharState::ATTACK_AIR, isAirAttacking);
	//states.addEdge(CharState::EVADE_START, CharState::ATTACK_AIR, isAirAttacking);
	//states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking);
	//states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking);

	//// Jumping.
	//auto isJumping{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP, true) };
	//	bool canJump{ isJumping && m_compPlayer.numRemainingJumps > 0 };

	//	if (canJump)
	//	{
	//		m_jumpSound->play(m_soundEngine);
	//		phys.isLockedDirection = false;
	//		phys.hasGravity = true;
	//		spr.isResetAnimation = true;
	//		m_compPlayer.numRemainingJumps--;
	//		phys.speed.y = character.jumpSpeed;

	//		// Don't allow for free evade if jumping from an evade.
	//		std::string prevState{ character.previousState };
	//		if (prevState == CharState::EVADE_START ||
	//			prevState == CharState::EVADE)
	//		{
	//			m_compPlayer.numRemainingEvades = glm::min(
	//				m_compPlayer.numRemainingEvades, m_compPlayer.numMaxEvades - 1);
	//		}
	//	}
	//	return canJump;
	//} };
	//states.addEdge(CharState::IDLE, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ALERT, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ALERT_STOP, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::RUN_START, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::RUN, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::RUN_STOP, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::TURN, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::CROUCH_STOP, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::JUMP_LAND, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ATTACK, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ATTACK2, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ATTACK3, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::EVADE_START, CharState::JUMP_ASCEND, isJumping);
	//states.addEdge(CharState::EVADE, CharState::JUMP_ASCEND, isJumping);

	//// Drop down.
	//auto isDroppingDown{ [this](int entityId) -> bool
	//{
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
	//	bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP) };
	//	return (isCrouching && isJumping && col.isCollidingGhost &&
	//		!col.isCollidingFloor && !col.isCollidingSlope);
	//} };
	//states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isDroppingDown);

	//// Evading.
	//auto isEvading{ [this](int entityId) -> bool
	//{
	//	bool isEvading{ m_inputManager->isKeyPressed(InputManager::INPUT_EVADE, true) };
	//	return (isEvading && m_compPlayer.numRemainingEvades > 0);
	//} };
	//states.addEdge(CharState::IDLE, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ALERT, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ALERT_STOP, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::RUN_START, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::RUN, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::RUN_STOP, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::TURN, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::CROUCH_STOP, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::JUMP_LAND, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ATTACK, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ATTACK2, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ATTACK3, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::JUMP_ASCEND, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::JUMP_PEAK, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::JUMP_DESCEND, CharState::EVADE_START, isEvading);
	//states.addEdge(CharState::ATTACK_AIR, CharState::EVADE_START, isEvading);

	//// Stop evading.
	//auto isEvadeTimerEndAndTurning{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };

	//	// Show turning animation if moving in opposite from the direction 
	//	// the player is facing.
	//	return (m_compPlayer.evadeTimer == 0.f && (isRunning &&
	//		((phys.scale.x < 0 && isRunningRight) || (phys.scale.x > 0 && isRunningLeft))));
	//} };
	//auto isEvadeTimerEnd{ [this](int entityId) -> bool
	//{
	//	return m_compPlayer.evadeTimer == 0.f;
	//} };
	//states.addEdge(CharState::EVADE_START, CharState::TURN, isEvadeTimerEndAndTurning);
	//states.addEdge(CharState::EVADE, CharState::TURN, isEvadeTimerEndAndTurning);
	//states.addEdge(CharState::EVADE_START, CharState::RUN, isEvadeTimerEnd);
	//states.addEdge(CharState::EVADE, CharState::RUN, isEvadeTimerEnd);

	//// Crouching.
	//auto isCrouching{ [this](int entityId) -> bool
	//{
	//	bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
	//	return isCrouching;
	//} };
	//states.addEdge(CharState::IDLE, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::ALERT, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::ALERT_STOP, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::RUN_START, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::RUN, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::RUN_STOP, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::TURN, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::CROUCH_STOP, CharState::CROUCH, isCrouching);
	//states.addEdge(CharState::JUMP_LAND, CharState::CROUCH, isCrouching);

	//// Stop crouching.
	//auto isStopCrouching{ [this](int entityId) -> bool
	//{
	//	bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
	//	return !isCrouching;
	//} };
	//states.addEdge(CharState::CROUCH, CharState::CROUCH_STOP, isStopCrouching);

	//// Start running.
	//auto isStartRunning{ [this](int entityId) -> bool
	//{
	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };

	//	return isRunning;
	//} };
	//auto isTurning{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };

	//	// Show turning animation if moving in opposite from the direction 
	//	// the player is facing.
	//	return isRunning &&
	//		((phys.scale.x < 0 && isRunningRight) || (phys.scale.x > 0 && isRunningLeft));
	//} };
	//states.addEdge(CharState::IDLE, CharState::TURN, isTurning);
	//states.addEdge(CharState::TURN, CharState::TURN, isTurning);
	//states.addEdge(CharState::RUN_START, CharState::TURN, isTurning);
	//states.addEdge(CharState::RUN, CharState::TURN, isTurning);
	//states.addEdge(CharState::RUN_STOP, CharState::TURN, isTurning);
	//states.addEdge(CharState::ALERT, CharState::TURN, isTurning);
	//states.addEdge(CharState::ALERT_STOP, CharState::TURN, isTurning);
	//states.addEdge(CharState::CROUCH_STOP, CharState::TURN, isTurning);
	//states.addEdge(CharState::JUMP_LAND, CharState::TURN, isTurning);

	//states.addEdge(CharState::IDLE, CharState::RUN_START, isStartRunning);
	//states.addEdge(CharState::RUN_STOP, CharState::RUN_START, isStartRunning);
	//states.addEdge(CharState::ALERT, CharState::RUN_START, isStartRunning);
	//states.addEdge(CharState::ALERT_STOP, CharState::RUN_START, isStartRunning);
	//states.addEdge(CharState::CROUCH_STOP, CharState::RUN_START, isStartRunning);
	//states.addEdge(CharState::JUMP_LAND, CharState::RUN_START, isStartRunning);

	//// Stop running.
	//auto isStopRunning{ [this](int entityId) -> bool
	//{
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	bool isStopRunningLeft{ m_inputManager->isKeyReleased(InputManager::INPUT_LEFT) };
	//	bool isStopRunningRight{ m_inputManager->isKeyReleased(InputManager::INPUT_RIGHT) };
	//	bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
	//	bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
	//	bool isRunning{ isRunningLeft != isRunningRight };

	//	return ((isStopRunningLeft && isStopRunningRight) || !isRunning);
	//} };
	//states.addEdge(CharState::RUN, CharState::RUN_STOP, isStopRunning);
	//states.addEdge(CharState::RUN_START, CharState::RUN_STOP, isStopRunning);
}

void EntityManager::createEnemy()
{
	std::shared_ptr<Prefab> enemyPrefab{ m_assetLoader->load<Prefab>("clamper") };
	int enemyId{ createEntity(enemyPrefab.get()) };

	GameComponent::Enemy &enemy = m_compEnemies[enemyId];

	GameComponent::Physics &phys = m_compPhysics[enemyId];
	phys.pos = glm::vec3(128.f, 800.f, 0.f);

	//GameComponent::Sprite &spr = m_compSprites[enemyId];
	//GameComponent::Collision &col = m_compCollisions[enemyId];

	//// Set up state machine.
	//GameComponent::Character &character{ m_compCharacters[enemyId] };
	//StateMachine &states{ character.states };

	//auto fallenEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	phys.hasFriction = true;
	//	character.fallenTimer = 3.f;
	//} };

	//auto fallenUpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	// If dead, make sprite translucent.
	//	if (GameComponent::isDead(character))
	//	{
	//		spr.a = 100;
	//	}
	//} };

	//auto runUpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	// If in the air, fix the frame to the last running frame.
	//	if (GameComponent::isInAir(phys, col))
	//	{
	//		spr.currentFrame = GameComponent::getNumSprites(spr) - 1;
	//		spr.currentFrameTime = 0.f;
	//	}

	//	// Move if on the ground.
	//	if (GameComponent::isOnGround(phys, col))
	//	{
	//		// Move in the other direction if colliding.
	//		if (col.isCollidingHorizontal)
	//		{
	//			if (enemy.isMovingLeft)
	//			{
	//				enemy.isMovingLeft = false;
	//				enemy.isMovingRight = true;
	//			}
	//			else if (enemy.isMovingRight)
	//			{
	//				enemy.isMovingLeft = true;
	//				enemy.isMovingRight = false;
	//			}
	//		}

	//		// If current speed is greater than the character's movement speed,
	//		// start applying friction to gradually bring it back down to
	//		// the character's movement speed.
	//		float currentSpeed{ glm::abs(phys.speed.x) };
	//		phys.hasFriction = (currentSpeed > character.movementSpeed);

	//		// Maintain maximum horizontal speed.
	//		float maxSpeed{ glm::max(character.movementSpeed, currentSpeed) };

	//		float dir{ 0.f };
	//		if (enemy.isMovingLeft)
	//			dir = -1.f;
	//		else if (enemy.isMovingRight)
	//			dir = 1.f;

	//		phys.speed.x += (dir * character.movementSpeed / 0.1f * m_deltaTime);
	//		phys.speed.x = glm::clamp(phys.speed.x, -maxSpeed, maxSpeed);
	//	}

	//	// Face the direction of movement if the enemy isn't targeting
	//	// the player.
	//	if (phys.speed.x != 0.f)
	//	{
	//		if (!enemy.isTargetingPlayer)
	//			phys.scale.x = glm::sign(phys.speed.x) * glm::abs(phys.scale.x);
	//		// Otherwise, face the player.
	//		else
	//		{
	//			glm::vec2 playerPos{ m_compPhysics[m_playerId].pos };
	//			if (playerPos.x < phys.pos.x)
	//				phys.scale.x = -glm::abs(phys.scale.x);
	//			else if (playerPos.x > phys.pos.x)
	//				phys.scale.x = glm::abs(phys.scale.x);
	//		}
	//	}

	//	// Target the player if the enemy can see them.
	//	if (!enemy.isTargetingPlayer)
	//	{
	//		glm::vec2 playerPos{ m_compPhysics[m_playerId].pos };
	//		bool canSeePlayerLeft{ playerPos.x >= phys.pos.x - enemy.targetRange.x && playerPos.x <= phys.pos.x };
	//		bool canSeePlayerRight{ playerPos.x <= phys.pos.x + enemy.targetRange.x && playerPos.x >= phys.pos.x };
	//		bool canSeePlayerDown{ playerPos.y >= phys.pos.y - enemy.targetRange.y };
	//		bool canSeePlayerUp{ playerPos.y <= phys.pos.y + enemy.targetRange.y };
	//		enemy.isTargetingPlayer = ((canSeePlayerLeft && phys.scale.x < 0) ||
	//			(canSeePlayerRight && phys.scale.x > 0)) &&
	//			canSeePlayerDown && canSeePlayerUp;
	//	}
	//} };

	//auto runExitAction{ [this](int entityId)
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	enemy.isMovingLeft = false;
	//	enemy.isMovingRight = false;
	//	phys.hasFriction = true;
	//} };

	//auto idleEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	GameComponent::setActionTimer(enemy);
	//} };

	//auto idleUpdateAction{ [this](int entityId)
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	glm::vec2 playerPos{ m_compPhysics[m_playerId].pos };
	//	bool canSeePlayerLeft{ playerPos.x >= phys.pos.x - enemy.targetRange.x };
	//	bool canSeePlayerRight{ playerPos.x <= phys.pos.x + enemy.targetRange.x  };
	//	bool canSeePlayerDown{ playerPos.y >= phys.pos.y - enemy.targetRange.y };
	//	bool canSeePlayerUp{ playerPos.y <= phys.pos.y + enemy.targetRange.y };

	//	// Target the player if the enemy can see them.
	//	if (!enemy.isTargetingPlayer)
	//	{
	//		enemy.isTargetingPlayer = ((canSeePlayerLeft && playerPos.x <= phys.pos.x && phys.scale.x < 0) ||
	//			(canSeePlayerRight && playerPos.x >= phys.pos.x && phys.scale.x > 0)) &&
	//			canSeePlayerDown && canSeePlayerUp;
	//	}
	//	// Lose track of the player if too far away.
	//	else
	//	{
	//		enemy.isTargetingPlayer = canSeePlayerLeft && canSeePlayerRight && 
	//			canSeePlayerDown && canSeePlayerUp;
	//	}
	//} };

	//auto attackEnterAction{ [this](int entityId)
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	float dir{ glm::sign(phys.scale.x) };
	//	phys.speed.x = 256.f * dir;
	//	phys.speed.y = 32.f;
	//	phys.hasFriction = false;
	//} };

	//auto attackExitAction{ [this](int entityId)
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };

	//	phys.hasFriction = true;
	//	enemy.attackTimer = enemy.attackDuration;
	//} };

	///*states.addState(CharState::IDLE, idleUpdateAction, idleEnterAction);
	//states.addState(CharState::RUN, runUpdateAction, [](int) {}, runExitAction);
	//states.addState(CharState::HURT);
	//states.addState(CharState::HURT_AIR);
	//states.addState(CharState::FALLEN, fallenUpdateAction, fallenEnterAction);
	//states.addState(CharState::ALERT);
	//states.addState(CharState::ATTACK, [](int) {}, attackEnterAction, attackExitAction);*/

	//// Hurt while on ground.
	//auto isHurting{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (character.hitStunTimer > 0.f && GameComponent::isOnGround(phys, col));
	//} };
	//states.addEdge(CharState::IDLE, CharState::HURT, isHurting);
	//states.addEdge(CharState::RUN, CharState::HURT, isHurting);
	//states.addEdge(CharState::ALERT, CharState::HURT, isHurting);
	//states.addEdge(CharState::ATTACK, CharState::HURT, isHurting);

	//// Stop hurting.
	//auto isStopHurting{ [this](int entityId) -> bool
	//{
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return character.hitStunTimer == 0.f;
	//} };
	//states.addEdge(CharState::HURT, CharState::IDLE, isStopHurting);

	//// Hurt while in the air.
	//auto isHurtingAir{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (character.hitStunTimer > 0.f && GameComponent::isInAir(phys, col));
	//} };
	//states.addEdge(CharState::RUN, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::HURT, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::FALLEN, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::ALERT, CharState::HURT_AIR, isHurtingAir);
	//states.addEdge(CharState::ATTACK, CharState::HURT_AIR, isHurtingAir);

	//// Fallen.
	//auto isFallen{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	return GameComponent::isOnGround(phys, col);
	//} };
	//states.addEdge(CharState::HURT_AIR, CharState::FALLEN, isFallen);

	//// Dead while on ground.
	//auto isDead{ [this](int entityId) -> bool
	//{
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return GameComponent::isDead(character);
	//} };
	//states.addEdge(CharState::HURT, CharState::FALLEN, isDead);

	//// Stop fallen.
	//auto isStopFallen{ [this](int entityId) -> bool
	//{
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return character.fallenTimer == 0.f && !GameComponent::isDead(character);
	//} };
	//states.addEdge(CharState::FALLEN, CharState::IDLE, isStopFallen);

	//// Falling.
	//auto isFalling{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	return (character.hitStunTimer == 0.f && GameComponent::isInAir(phys, col));
	//} };
	//states.addEdge(CharState::IDLE, CharState::RUN, isFalling);

	//// Stop moving.
	//auto isStopMoving{ [this](int entityId) -> bool
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	bool isLanding{ GameComponent::isOnGround(phys, col) && 
	//		!enemy.isMovingLeft && !enemy.isMovingRight };
	//	bool isStoppedPatrolling{ (enemy.isMovingLeft || enemy.isMovingRight) && 
	//		enemy.actionTimer == 0.f };

	//	return isLanding || isStoppedPatrolling;
	//} };
	//states.addEdge(CharState::RUN, CharState::IDLE, isStopMoving);

	//// Start moving.
	//auto isMoving{ [this](int entityId) -> bool
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	if (enemy.isTargetingPlayer && enemy.attackTimer == 0.f)
	//	{
	//		// Move towards player.
	//		glm::vec2 playerPos{ m_compPhysics[m_playerId].pos };
	//		if (playerPos.x < phys.pos.x)
	//		{
	//			enemy.isMovingLeft = true;
	//			enemy.isMovingRight = false;
	//		}
	//		else if (playerPos.x > phys.pos.x)
	//		{
	//			enemy.isMovingLeft = false;
	//			enemy.isMovingRight = true;
	//		}

	//		// Respond twice as quickly when targeting player.
	//		GameComponent::setActionTimer(enemy, 0.5f);

	//		return true;
	//	}
	//	else if (enemy.actionTimer == 0.f)
	//	{
	//		// Move in random direction.
	//		if (rand() % 2 == 0)
	//		{
	//			enemy.isMovingLeft = true;
	//			enemy.isMovingRight = false;
	//		}
	//		else
	//		{
	//			enemy.isMovingLeft = false;
	//			enemy.isMovingRight = true;
	//		}
	//		GameComponent::setActionTimer(enemy);

	//		return true;
	//	}

	//	return false;
	//} };
	//states.addEdge(CharState::IDLE, CharState::RUN, isMoving);

	//// Alert.
	//auto isAlert{ [this](int entityId) -> bool
	//{
	//	GameComponent::Enemy &enemy{ m_compEnemies[entityId] };
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };
	//	GameComponent::Character &character{ m_compCharacters[entityId] };

	//	if (enemy.isTargetingPlayer && enemy.attackTimer == 0.f && 
	//		GameComponent::isOnGround(phys, col))
	//	{
	//		glm::vec2 playerPos{ m_compPhysics[m_playerId].pos };
	//		bool isInRangeLeft{ playerPos.x >= phys.pos.x - enemy.attackRange.x };
	//		bool isInRangeRight{ playerPos.x <= phys.pos.x + enemy.attackRange.x };
	//		bool isInRangeDown{ playerPos.y >= phys.pos.y - enemy.attackRange.y };
	//		bool inInRangeUp{ playerPos.y <= phys.pos.y + enemy.attackRange.y };

	//		if (isInRangeLeft && isInRangeRight &&
	//			isInRangeDown && inInRangeUp)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//} };
	//states.addEdge(CharState::IDLE, CharState::ALERT, isAlert);
	//states.addEdge(CharState::RUN, CharState::ALERT, isAlert);

	//// Animation end transitions.
	//auto isAnimationEnd{ [this](int entityId) -> bool
	//{
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };

	//	float frameDuration{ GameComponent::getFrameDuration(spr) };
	//	int numSprites{ GameComponent::getNumSprites(spr) };
	//	return (spr.currentFrameTime >= frameDuration &&
	//		(!spr.currentSprite.isLooping && spr.currentFrame == numSprites - 1));
	//} };
	//states.addEdge(CharState::ALERT, CharState::ATTACK, isAnimationEnd);

	//auto isAttackEnd{ [this](int entityId) -> bool
	//{
	//	GameComponent::Physics &phys{ m_compPhysics[entityId] };
	//	GameComponent::Sprite &spr{ m_compSprites[entityId] };
	//	GameComponent::Collision &col{ m_compCollisions[entityId] };

	//	float frameDuration{ GameComponent::getFrameDuration(spr) };
	//	int numSprites{ GameComponent::getNumSprites(spr) };
	//	bool isAnimationEnd{ spr.currentFrameTime >= frameDuration &&
	//		(!spr.currentSprite.isLooping && spr.currentFrame == numSprites - 1) };
	//	return isAnimationEnd && GameComponent::isOnGround(phys, col);
	//} };
	//states.addEdge(CharState::ATTACK, CharState::IDLE, isAttackEnd);
}

//void GameEngine::createNewEntities()
//{
//	// The number of new sprites to create this frame.
//	int numNewEntities{ (int)(m_deltaTime * NUM_ENTITIES_PER_SECOND) };
//	if (numNewEntities > (int)(SECONDS_PER_FRAME * NUM_ENTITIES_PER_SECOND))
//		numNewEntities = (int)(SECONDS_PER_FRAME * NUM_ENTITIES_PER_SECOND);
//
//	// Generate the new sprites with random values.
//	for (int i = 0; i < numNewEntities; i++)
//	{
//		int entityIndex{ createEntity({ 
//			GameComponent::COMPONENT_PHYSICS,
//			GameComponent::COMPONENT_SPRITE,
//		}) };
//
//		GameComponent::Physics &phys = m_compPhysics[entityIndex];
//		phys.pos = glm::vec3(0, 5, -20.0f);
//
//		float spread{ 1.5f };
//		glm::vec3 mainDir{ glm::vec3(0.0f, 10.0f, 0.0f) };
//		glm::vec3 randomDir{ glm::vec3(
//			(rand() % 2000 - 1000.0f) / 1000.0f,
//			(rand() % 2000 - 1000.0f) / 1000.0f,
//			(rand() % 2000 - 1000.0f) / 1000.0f
//		) };
//
//		phys.speed = mainDir + randomDir * spread;
//		phys.scale = glm::vec2((rand() % 1000) / 2000.0f + 0.1f);
//
//		GameComponent::Sprite &spr = m_compSprites[entityIndex];
//		spr.duration = 5.0f;
//		spr.r = rand() % 256;
//		spr.g = rand() % 256;
//		spr.b = rand() % 256;
//		spr.a = 255;
//
//		spr.spriteSheet = m_playerTexture.get();
//		spr.spriteSheet->setAnimation("run", spr);
//	}
//}

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
			sound->play(m_soundEngine);
		});
	m_lua.set_function("playAttackSound", [this](int entityId)
		{
			GameComponent::Attack &atk{ m_compAttacks[entityId] };
			if (atk.pattern.attackSound != nullptr)
				atk.pattern.attackSound->play(m_soundEngine);
		});
	m_lua.set_function("playEvadeSound", [this](int entityId)
		{
			if (m_compPlayer.evadeSound != nullptr)
				m_compPlayer.evadeSound->play(m_soundEngine);
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
	m_lua.set_function("isDead", [this](int entityId)
		{
			return GameComponent::isDead(m_compCharacters[entityId]);
		});
	m_lua.set_function("setFallenTimer", [this](int entityId, float value)
		{
			m_compCharacters[entityId].fallenTimer = value;
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
			return m_compAttacks[entityId].pattern.frameRange.x;
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