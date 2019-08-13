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

#include "AttackCollisionSystem.h"
#include "AttackSystem.h"
#include "CharacterSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSystem.h"
#include "RoomCollisionSystem.h"
#include "SpriteSystem.h"

#include <iostream>

using json = nlohmann::json;

namespace
{
	// Component labels for prefabs.
	const std::string COMPONENT_SPRITE{ "sprite" };
	const std::string COMPONENT_PHYSICS{ "physics" };
	const std::string COMPONENT_PLAYER{ "player" };
	const std::string COMPONENT_COLLISION{ "collision" };
	const std::string COMPONENT_WEAPON{ "weapon" };
	const std::string COMPONENT_ATTACK{ "attack" };
	const std::string COMPONENT_CHARACTER{ "character" };
	const std::unordered_map<std::string, GameComponent::ComponentType> LABEL_TO_COMPONENT{
		{ COMPONENT_SPRITE, GameComponent::COMPONENT_SPRITE },
		{ COMPONENT_PHYSICS, GameComponent::COMPONENT_PHYSICS },
		{ COMPONENT_PLAYER, GameComponent::COMPONENT_PLAYER },
		{ COMPONENT_COLLISION, GameComponent::COMPONENT_COLLISION },
		{ COMPONENT_WEAPON, GameComponent::COMPONENT_WEAPON },
		{ COMPONENT_ATTACK, GameComponent::COMPONENT_ATTACK },
		{ COMPONENT_CHARACTER, GameComponent::COMPONENT_CHARACTER },
	};

	const std::string PROPERTY_COMPONENTS{ "components" };
	const std::string PROPERTY_SPRITESHEET{ "spritesheet" };
	const std::string PROPERTY_EVADEDURATION{ "evadeDuration" };
	const std::string PROPERTY_BOXES{ "boxes" };
	const std::string PROPERTY_HALFSIZE{ "halfSize" };
	const std::string PROPERTY_OFFSET{ "offset" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_HEALTH{ "health" };
	const std::string PROPERTY_RESOURCE{ "resource" };
	const std::string PROPERTY_ATTACKPATTERNS{ "attackPatterns" };
	const std::string PROPERTY_TYPE{ "type" };
	const std::string PROPERTY_FRAMERANGE{ "frameRange" };
	const std::string PROPERTY_START{ "start" };
	const std::string PROPERTY_END{ "end" };
	const std::string PROPERTY_COMBOFRAME{ "comboFrame" };
	const std::string PROPERTY_HITSOUND{ "hitSound" };
	const std::string PROPERTY_HITEFFECT{ "hitEffect" };
	const std::string PROPERTY_COOLDOWN{ "cooldown" };
	const std::string PROPERTY_DAMAGE{ "damage" };
	const std::string PROPERTY_KNOCKBACK{ "knockback" };
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

	// TODO: replace these hardcoded resources.
	m_effectsTexture = assetLoader->load<SpriteSheet>("effects");
	m_jumpSound = assetLoader->load<Sound>("jump");
	m_evadeSound = assetLoader->load<Sound>("evade");
	m_attackSound = assetLoader->load<Sound>("serah_attack");
	m_hitSound = assetLoader->load<Sound>("hit");
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
		game, m_compPhysics, m_compCollisions, m_compCharacters));
	m_gameSystems.emplace_back(std::make_unique<PlayerSystem>(*this,
		m_compPlayer, m_compPhysics, m_compCollisions));
	m_gameSystems.emplace_back(std::make_unique<CharacterSystem>(*this,
		m_compSprites, m_compWeapons, m_compCollisions, m_compAttacks,
		m_compCharacters));

	m_debugSystem = std::make_unique<DebugSystem>(*this, uRenderer,
		m_compPhysics, m_compCollisions, m_compAttacks);
}

EntityManager::~EntityManager()
{

}

void EntityManager::update(float deltaTime, AssetLoader *assetLoader,
	UIRenderer *uRenderer, TextRenderer *tRenderer, bool isDebugMode)
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

	// Only process debug system if in debug mode.
	if (isDebugMode)
	{
		m_debugSystem->update(deltaTime, m_numEntities, m_entities);
	}

	// Reset overlaps list.
	m_collisions.clear();

	// Delete all flagged entities.
	deleteFlaggedEntities();

	// Update the hud with entity values.
	const GameComponent::Character &playerChar{ m_compCharacters[m_playerId] };
	uRenderer->updateHud(assetLoader, tRenderer, playerChar.health, 
		playerChar.maxHealth, playerChar.resource, playerChar.maxResource,
		m_compPlayer.skillTimers);
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
					initializePlayer(component);
				else if (type == COMPONENT_COLLISION)
					initializeCollision(id, component);
				else if (type == COMPONENT_WEAPON)
					initializeWeapon(id, component);
				else if (type == COMPONENT_ATTACK)
					initializeAttack(id, component);
				else if (type == COMPONENT_CHARACTER)
					initializeCharacter(id, component);
			}
		}
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

void EntityManager::initializePlayer(const nlohmann::json &json)
{
	GameComponent::Player &player{ m_compPlayer };
	if (JSONUtilities::hasEntry(PROPERTY_EVADEDURATION, json))
	{
		float evadeDuration{ json.at(PROPERTY_EVADEDURATION).get<float>() };
		player.evadeDuration = evadeDuration;
	}
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

void EntityManager::deleteFlaggedEntities()
{
	for (int id : m_entitiesToDelete)
	{
		// Reset the entity and swap with the last entity to
		// keep the entities array tightly packed.
		int lastIndex = m_numEntities - 1;
		unsigned long lastMask = m_entities[lastIndex];
		m_entities[id] = lastMask;
		m_entities[lastIndex] = GameComponent::COMPONENT_NONE;

		// Swap all its components too.
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_PHYSICS))
		{
			m_compPhysics[id] = m_compPhysics[lastIndex];
			m_compPhysics[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_SPRITE))
		{
			m_compSprites[id] = m_compSprites[lastIndex];
			m_compSprites[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_PLAYER))
		{
			m_compPlayer = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_COLLISION))
		{
			m_compCollisions[id] = m_compCollisions[lastIndex];
			m_compCollisions[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_WEAPON))
		{
			m_compWeapons[id] = m_compWeapons[lastIndex];
			m_compWeapons[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_ATTACK))
		{
			m_compAttacks[id] = m_compAttacks[lastIndex];
			m_compAttacks[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_ENEMY))
		{
			m_compEnemies[id] = m_compEnemies[lastIndex];
			m_compEnemies[lastIndex] = {};
		}
		if (GameComponent::hasComponent(lastMask, GameComponent::COMPONENT_CHARACTER))
		{
			m_compCharacters[id] = m_compCharacters[lastIndex];
			m_compCharacters[lastIndex] = {};
		}

		m_numEntities--;
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
	GameComponent::Sprite &spr = m_compSprites[m_playerId];
	GameComponent::Weapon &wp = m_compWeapons[m_playerId];
	GameComponent::Collision &col = m_compCollisions[m_playerId];
	GameComponent::Attack &atk = m_compAttacks[m_playerId];
	GameComponent::Character &character = m_compCharacters[m_playerId];

	// Set up state machine.
	StateMachine &states{ character.states };

	auto enableFriction{ [&phys]()
	{
		phys.hasFriction = true;
	} };

	auto enableGravity{ [&phys]()
	{
		phys.hasGravity = true;
	} };

	auto evadeUpdateAction{ [&phys, &col]()
	{
		// If in the air, reset vertical speed and disable gravity.
		if (GameComponent::isInAir(phys, col))
		{
			phys.speed.y = 0.f;
			phys.hasGravity = false;
		}
		else
		{
			phys.hasGravity = true;
		}
	} };

	auto evadeEnterAction{ [&phys, &character, this]()
	{
		// By default, just evade in the direction that
		// the player is facing.
		float dir{ phys.scale.x > 0 ? 1.f : -1.f };

		// If left/right is held down, then evade in that direction
		// instead.
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };
		if (isRunning)
		{
			if (isRunningLeft)
				dir = -1.f;
			else if (isRunningRight)
				dir = 1.f;
		}

		m_compPlayer.numRemainingEvades--;
		phys.speed.x = dir * 1.7f * character.movementSpeed;
		phys.scale.x = glm::abs(phys.scale.x) * dir;
		phys.hasFriction = false;
		phys.isLockedDirection = false;

		// Set the evade timer.
		m_compPlayer.evadeTimer = m_compPlayer.evadeDuration;

		// Create smoke effect.
		glm::vec3 effectPos{ phys.pos };
		effectPos.x -= (dir * 20.f);
		effectPos.y -= 4.f;
		createEffect(EffectType::EVADE_SMOKE, effectPos, phys.scale, 
			255, 255, 255, 180);

		m_evadeSound->play(m_soundEngine);
	} };

	auto runUpdateAction{ [&phys, &character, this]()
	{
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		// Stopping while turning.
		bool isStopRunningLeft{ m_inputManager->isKeyReleased(InputManager::INPUT_LEFT) };
		bool isStopRunningRight{ m_inputManager->isKeyReleased(InputManager::INPUT_RIGHT) };
		bool isTurning{ character.states.getState() == CharState::TURN };
		if (isTurning && ((isStopRunningLeft && isStopRunningRight) || !isRunning))
		{
			phys.hasFriction = true;
			return;
		}

		// If current speed is greater than the character's movement speed,
		// start applying friction to gradually bring it back down to
		// the character's movement speed.
		float currentSpeed{ glm::abs(phys.speed.x) };
		phys.hasFriction = (currentSpeed > character.movementSpeed);

		// Maintain maximum horizontal speed.
		float maxSpeed{ glm::max(character.movementSpeed, currentSpeed) };

		float dir{ 0.f };
		if (isRunning)
		{
			if (isRunningLeft)
				dir = -1.f;
			else if (isRunningRight)
				dir = 1.f;
		}

		phys.speed.x += (dir * character.movementSpeed / 0.1f * m_deltaTime);
		phys.speed.x = glm::clamp(phys.speed.x, -maxSpeed, maxSpeed);

		if (dir != 0.f && !phys.isLockedDirection)
			phys.scale.x = glm::abs(phys.scale.x) * dir;
	} };

	auto dropDownEnterAction{ [&phys, &spr, &col, this]()
	{
		bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
		bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP) };
		if (isCrouching && isJumping && col.isCollidingGhost &&
			!col.isCollidingFloor && !col.isCollidingSlope)
		{
			spr.isResetAnimation = true;
			m_compPlayer.numRemainingJumps--;
			phys.speed.y = 0.f;
			phys.pos.y--;
		}
	} };

	auto attackAirEnterAction{ [&phys]()
	{
		phys.isLockedDirection = true;
	} };

	auto jumpLandEnterAction{ [&phys]()
	{
		phys.isLockedDirection = false;
		phys.hasFriction = true;
	} };

	auto turnEnterAction{ [&spr]()
	{
		spr.isResetAnimation = true;
	} };

	auto skill1EnterAction{ [&phys, &col, &atk, this]()
	{
		m_compPlayer.skillTimers[0] = atk.pattern.cooldown;
		phys.speed.x = 0.f;
		if (GameComponent::isInAir(phys, col))
			phys.speed.y = 0.f;
		phys.hasGravity = false;
	} };

	auto skill1ExitAction{ [&phys]()
	{
		phys.hasGravity = true;
	} };

	auto skill1UpdateAction{ [&spr, &phys, &atk]()
	{
		if (spr.currentFrame == atk.pattern.frameRange.x && !phys.hasGravity)
		{
			float dir{ phys.scale.x > 0 ? 1.f : -1.f };
			phys.speed.x = dir * 16.f;
			phys.speed.y = atk.pattern.knockback.y - 16.f;
			phys.isLockedDirection = true;
			phys.hasGravity = true;
		}
	} };

	states.addState(CharState::IDLE);
	states.addState(CharState::RUN, runUpdateAction);
	states.addState(CharState::JUMP_ASCEND, runUpdateAction);
	states.addState(CharState::JUMP_PEAK, runUpdateAction, dropDownEnterAction);
	states.addState(CharState::JUMP_DESCEND, runUpdateAction);
	states.addState(CharState::JUMP_LAND, []() {}, jumpLandEnterAction);
	states.addState(CharState::RUN_START, runUpdateAction);
	states.addState(CharState::RUN_STOP, enableFriction);
	states.addState(CharState::ALERT);
	states.addState(CharState::ALERT_STOP);
	states.addState(CharState::TURN, runUpdateAction, turnEnterAction);
	states.addState(CharState::CROUCH, []() {}, enableFriction);
	states.addState(CharState::CROUCH_STOP);
	states.addState(CharState::ATTACK, []() {}, enableFriction);
	states.addState(CharState::ATTACK2, []() {}, enableFriction);
	states.addState(CharState::ATTACK3, []() {}, enableFriction);
	states.addState(CharState::ATTACK_AIR, runUpdateAction, attackAirEnterAction);
	states.addState(CharState::ATTACK_CROUCH);
	states.addState(CharState::EVADE_START, evadeUpdateAction, evadeEnterAction, enableGravity);
	states.addState(CharState::EVADE, evadeUpdateAction, []() {}, enableGravity);
	states.addState(CharState::SKILL1, skill1UpdateAction, skill1EnterAction, skill1ExitAction);

	// Animation end transitions.
	auto isAnimationEnd{ [&spr]() -> bool
	{
		float frameDuration{ GameComponent::getFrameDuration(spr) };
		int numSprites{ GameComponent::getNumSprites(spr) };
		return (spr.currentFrameTime >= frameDuration &&
			(!spr.currentSprite.isLooping && spr.currentFrame == numSprites - 1));
	} };
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_DESCEND, isAnimationEnd);
	states.addEdge(CharState::JUMP_LAND, CharState::CROUCH_STOP, isAnimationEnd);
	states.addEdge(CharState::CROUCH_STOP, CharState::IDLE, isAnimationEnd);
	states.addEdge(CharState::RUN_START, CharState::RUN, isAnimationEnd);
	states.addEdge(CharState::RUN_STOP, CharState::ALERT, isAnimationEnd);
	states.addEdge(CharState::TURN, CharState::RUN_START, isAnimationEnd);
	states.addEdge(CharState::ALERT, CharState::ALERT_STOP, isAnimationEnd);
	states.addEdge(CharState::ALERT_STOP, CharState::IDLE, isAnimationEnd);
	states.addEdge(CharState::ATTACK, CharState::ALERT_STOP, isAnimationEnd);
	states.addEdge(CharState::ATTACK2, CharState::ALERT_STOP, isAnimationEnd);
	states.addEdge(CharState::ATTACK3, CharState::ALERT_STOP, isAnimationEnd);
	states.addEdge(CharState::ATTACK_CROUCH, CharState::CROUCH, isAnimationEnd);
	states.addEdge(CharState::EVADE_START, CharState::EVADE, isAnimationEnd);
	states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_ASCEND, isAnimationEnd);

	// Falling.
	auto isFalling{ [&phys, &col, &character]() -> bool
	{
		return (phys.speed.y < 0 && GameComponent::isInAir(phys, col) &&
			character.states.getState() != CharState::JUMP_DESCEND);
	} };
	states.addEdge(CharState::IDLE, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::RUN_START, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::RUN, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::RUN_STOP, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::TURN, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::CROUCH_STOP, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::ATTACK, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::ATTACK_CROUCH, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::ATTACK2, CharState::JUMP_PEAK, isFalling);
	states.addEdge(CharState::ATTACK3, CharState::JUMP_PEAK, isFalling);

	// Landing.
	auto isLanding{ [&phys, &col]() -> bool
	{
		return GameComponent::isOnGround(phys, col);
	} };
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_LAND, isLanding);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_LAND, isLanding);
	states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_LAND, isLanding);

	// Skill 1.
	auto isSkill1{ [this]() -> bool
	{
		return m_inputManager->isKeyPressed(InputManager::INPUT_SKILL1) &&
			m_compPlayer.skillTimers[0] == 0.f;
	} };
	states.addEdge(CharState::IDLE, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ALERT, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ALERT_STOP, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::RUN_START, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::RUN, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::RUN_STOP, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::TURN, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::CROUCH_STOP, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::JUMP_LAND, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::JUMP_ASCEND, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::JUMP_PEAK, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::JUMP_DESCEND, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ATTACK, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ATTACK2, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ATTACK3, CharState::SKILL1, isSkill1);
	states.addEdge(CharState::ATTACK_AIR, CharState::SKILL1, isSkill1);

	auto isSkill1End{ [&spr, &phys, &atk]() -> bool
	{
		return (phys.speed.y < 0.f &&
			spr.currentFrame > atk.pattern.frameRange.x);
	} };
	states.addEdge(CharState::SKILL1, CharState::JUMP_ASCEND, isSkill1End);

	// Attacking.
	auto isAttacking{ [this]() -> bool
	{
		bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) };
		if (isAttacking)
		{
			m_attackSound->play(m_soundEngine);
		}
		return isAttacking;
	} };
	states.addEdge(CharState::IDLE, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::ALERT, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::ALERT_STOP, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::RUN_START, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::RUN, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::RUN_STOP, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::TURN, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::CROUCH, CharState::ATTACK_CROUCH, isAttacking);
	states.addEdge(CharState::CROUCH_STOP, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::JUMP_LAND, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::JUMP_ASCEND, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::JUMP_PEAK, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::JUMP_DESCEND, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking);

	auto isAttackCombo{ [&spr, &atk, this]() -> bool
	{
		bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) && 
			spr.currentFrame > atk.pattern.comboFrame };
		if (isAttacking)
		{
			m_attackSound->play(m_soundEngine);
		}
		return isAttacking;
	} };
	states.addEdge(CharState::ATTACK, CharState::ATTACK2, isAttackCombo);
	states.addEdge(CharState::ATTACK2, CharState::ATTACK3, isAttackCombo);

	auto isAirAttacking{ [&phys, &col, this]() -> bool
	{
		bool isAttacking{ m_inputManager->isKeyPressed(InputManager::INPUT_ATTACK) };
		return (isAttacking && GameComponent::isInAir(phys, col));
	} };
	states.addEdge(CharState::EVADE, CharState::ATTACK_AIR, isAirAttacking);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK_AIR, isAirAttacking);
	states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking);

	// Jumping.
	auto isJumping{ [&spr, &phys, &character, this]() -> bool
	{
		bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP, true) };
		bool canJump{ isJumping && m_compPlayer.numRemainingJumps > 0 };

		if (canJump)
		{
			m_jumpSound->play(m_soundEngine);
			phys.isLockedDirection = false;
			phys.hasGravity = true;
			spr.isResetAnimation = true;
			m_compPlayer.numRemainingJumps--;
			phys.speed.y = character.jumpSpeed;

			// Don't allow for free evade if jumping from an evade.
			std::string prevState{ character.previousState };
			if (prevState == CharState::EVADE_START ||
				prevState == CharState::EVADE)
			{
				m_compPlayer.numRemainingEvades = glm::min(
					m_compPlayer.numRemainingEvades, m_compPlayer.numMaxEvades - 1);
			}
		}
		return canJump;
	} };
	states.addEdge(CharState::IDLE, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ALERT, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ALERT_STOP, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::RUN_START, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::RUN, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::RUN_STOP, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::TURN, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::CROUCH_STOP, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::JUMP_LAND, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ATTACK, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ATTACK2, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ATTACK3, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::EVADE_START, CharState::JUMP_ASCEND, isJumping);
	states.addEdge(CharState::EVADE, CharState::JUMP_ASCEND, isJumping);

	// Drop down.
	auto isDroppingDown{ [&col, this]() -> bool
	{
		bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
		bool isJumping{ m_inputManager->isKeyPressed(InputManager::INPUT_JUMP) };
		return (isCrouching && isJumping && col.isCollidingGhost &&
			!col.isCollidingFloor && !col.isCollidingSlope);
	} };
	states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isDroppingDown);

	// Evading.
	auto isEvading{ [this]() -> bool
	{
		bool isEvading{ m_inputManager->isKeyPressed(InputManager::INPUT_EVADE, true) };
		return (isEvading && m_compPlayer.numRemainingEvades > 0);
	} };
	states.addEdge(CharState::IDLE, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ALERT, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ALERT_STOP, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::RUN_START, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::RUN, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::RUN_STOP, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::TURN, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::CROUCH_STOP, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::JUMP_LAND, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ATTACK, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ATTACK2, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ATTACK3, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::JUMP_ASCEND, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::JUMP_PEAK, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::JUMP_DESCEND, CharState::EVADE_START, isEvading);
	states.addEdge(CharState::ATTACK_AIR, CharState::EVADE_START, isEvading);

	// Stop evading.
	auto isEvadeTimerEndAndTurning{ [&phys, this]() -> bool
	{
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		// Show turning animation if moving in opposite from the direction 
		// the player is facing.
		return (m_compPlayer.evadeTimer == 0.f && (isRunning &&
			((phys.scale.x < 0 && isRunningRight) || (phys.scale.x > 0 && isRunningLeft))));
	} };
	auto isEvadeTimerEnd{ [this]() -> bool
	{
		return m_compPlayer.evadeTimer == 0.f;
	} };
	states.addEdge(CharState::EVADE_START, CharState::TURN, isEvadeTimerEndAndTurning);
	states.addEdge(CharState::EVADE, CharState::TURN, isEvadeTimerEndAndTurning);
	states.addEdge(CharState::EVADE_START, CharState::RUN, isEvadeTimerEnd);
	states.addEdge(CharState::EVADE, CharState::RUN, isEvadeTimerEnd);

	// Crouching.
	auto isCrouching{ [this]() -> bool
	{
		bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
		return isCrouching;
	} };
	states.addEdge(CharState::IDLE, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::ALERT, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::ALERT_STOP, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::RUN_START, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::RUN, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::RUN_STOP, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::TURN, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::CROUCH_STOP, CharState::CROUCH, isCrouching);
	states.addEdge(CharState::JUMP_LAND, CharState::CROUCH, isCrouching);

	// Stop crouching.
	auto isStopCrouching{ [this]() -> bool
	{
		bool isCrouching{ m_inputManager->isKeyPressing(InputManager::INPUT_DOWN) };
		return !isCrouching;
	} };
	states.addEdge(CharState::CROUCH, CharState::CROUCH_STOP, isStopCrouching);

	// Start running.
	auto isStartRunning{ [this]() -> bool
	{
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return isRunning;
	} };
	auto isTurning{ [&character, &spr, &phys, this]() -> bool
	{
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		// Show turning animation if moving in opposite from the direction 
		// the player is facing.
		return isRunning &&
			((phys.scale.x < 0 && isRunningRight) || (phys.scale.x > 0 && isRunningLeft));
	} };
	states.addEdge(CharState::IDLE, CharState::TURN, isTurning);
	states.addEdge(CharState::TURN, CharState::TURN, isTurning);
	states.addEdge(CharState::RUN_START, CharState::TURN, isTurning);
	states.addEdge(CharState::RUN, CharState::TURN, isTurning);
	states.addEdge(CharState::RUN_STOP, CharState::TURN, isTurning);
	states.addEdge(CharState::ALERT, CharState::TURN, isTurning);
	states.addEdge(CharState::ALERT_STOP, CharState::TURN, isTurning);
	states.addEdge(CharState::CROUCH_STOP, CharState::TURN, isTurning);
	states.addEdge(CharState::JUMP_LAND, CharState::TURN, isTurning);

	states.addEdge(CharState::IDLE, CharState::RUN_START, isStartRunning);
	states.addEdge(CharState::RUN_STOP, CharState::RUN_START, isStartRunning);
	states.addEdge(CharState::ALERT, CharState::RUN_START, isStartRunning);
	states.addEdge(CharState::ALERT_STOP, CharState::RUN_START, isStartRunning);
	states.addEdge(CharState::CROUCH_STOP, CharState::RUN_START, isStartRunning);
	states.addEdge(CharState::JUMP_LAND, CharState::RUN_START, isStartRunning);

	// Stop running.
	auto isStopRunning{ [&character, this]() -> bool
	{
		bool isStopRunningLeft{ m_inputManager->isKeyReleased(InputManager::INPUT_LEFT) };
		bool isStopRunningRight{ m_inputManager->isKeyReleased(InputManager::INPUT_RIGHT) };
		bool isRunningLeft{ m_inputManager->isKeyPressing(InputManager::INPUT_LEFT) };
		bool isRunningRight{ m_inputManager->isKeyPressing(InputManager::INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return ((isStopRunningLeft && isStopRunningRight) || !isRunning);
	} };
	states.addEdge(CharState::RUN, CharState::RUN_STOP, isStopRunning);
	states.addEdge(CharState::RUN_START, CharState::RUN_STOP, isStopRunning);
}

void EntityManager::createEnemy()
{
	std::shared_ptr<Prefab> enemyPrefab{ m_assetLoader->load<Prefab>("clamper") };
	int enemyId{ createEntity(enemyPrefab.get()) };

	GameComponent::Physics &phys = m_compPhysics[enemyId];
	phys.pos = glm::vec3(128.f, 800.f, 0.f);

	GameComponent::Sprite &spr = m_compSprites[enemyId];
	GameComponent::Collision &col = m_compCollisions[enemyId];

	// Set up state machine.
	GameComponent::Character &character{ m_compCharacters[enemyId] };
	StateMachine &states{ character.states };

	auto fallenEnterAction{ [&character]()
	{
		character.fallenTimer = 3.f;
	} };

	auto runUpdateAction{ [&spr, &col, &phys]()
	{
		// If in the air, fix the frame to the last running frame.
		if (GameComponent::isInAir(phys, col))
		{
			spr.currentFrame = GameComponent::getNumSprites(spr) - 1;
			spr.currentFrameTime = 0.f;
		}
	} };

	states.addState(CharState::IDLE);
	states.addState(CharState::RUN, runUpdateAction);
	states.addState(CharState::HURT);
	states.addState(CharState::HURT_AIR);
	states.addState(CharState::FALLEN, []() {}, fallenEnterAction);

	// Hurt while on ground.
	auto isHurting{ [&character, &col, &phys]() -> bool
	{
		return (character.hitStunTimer > 0.f && GameComponent::isOnGround(phys, col));
	} };
	states.addEdge(CharState::IDLE, CharState::HURT, isHurting);
	states.addEdge(CharState::RUN, CharState::HURT, isHurting);

	// Stop hurting.
	auto isStopHurting{ [&character, &col]() -> bool
	{
		return character.hitStunTimer == 0.f;
	} };
	states.addEdge(CharState::HURT, CharState::IDLE, isStopHurting);

	// Hurt while in the air.
	auto isHurtingAir{ [&character, &col, &phys]() -> bool
	{
		return (character.hitStunTimer > 0.f && GameComponent::isInAir(phys, col));
	} };
	states.addEdge(CharState::IDLE, CharState::HURT_AIR, isHurtingAir);
	states.addEdge(CharState::RUN, CharState::HURT_AIR, isHurtingAir);
	states.addEdge(CharState::HURT, CharState::HURT_AIR, isHurtingAir);
	states.addEdge(CharState::FALLEN, CharState::HURT_AIR, isHurtingAir);

	// Fallen.
	auto isFallen{ [&col, &phys]() -> bool
	{
		return GameComponent::isOnGround(phys, col);
	} };
	states.addEdge(CharState::HURT_AIR, CharState::FALLEN, isFallen);

	// Stop fallen.
	auto isStopFallen{ [&character, &col]() -> bool
	{
		return character.fallenTimer == 0.f;
	} };
	states.addEdge(CharState::FALLEN, CharState::IDLE, isStopFallen);

	// Moving.
	auto isMoving{ [&character, &col, &phys]() -> bool
	{
		return (character.hitStunTimer == 0.f &&
			(phys.speed.x != 0.f || GameComponent::isInAir(phys, col)));
	} };
	states.addEdge(CharState::IDLE, CharState::RUN, isMoving);

	// Stop moving.
	auto isStopMoving{ [&col, &phys]() -> bool
	{
		return (GameComponent::isOnGround(phys, col) && phys.speed.x == 0.f);
	} };
	states.addEdge(CharState::RUN, CharState::IDLE, isStopMoving);
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