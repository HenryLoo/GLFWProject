#include "EntityManager.h"

#include "CharStates.h"
#include "EffectTypes.h"
#include "EntityConstants.h"
#include "GameEngine.h"
#include "SpriteSheet.h"

#include "AttackCollisionSystem.h"
#include "AttackSystem.h"
#include "CharacterSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSystem.h"
#include "RoomCollisionSystem.h"
#include "SpriteSystem.h"

#include <iostream>

EntityManager::EntityManager(GameEngine &game) :
	m_game(game)
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
	createEnemy();
	createPlayer();

	// Initialze game systems.
	m_gameSystems.emplace_back(std::make_unique<AttackSystem>(*this,
		m_compSprites, m_compAttacks));
	m_gameSystems.emplace_back(std::make_unique<AttackCollisionSystem>(*this,
		m_compPhysics, m_compSprites, m_compCollisions, m_compAttacks,
		m_compCharacters));
	m_gameSystems.emplace_back(std::make_unique<PhysicsSystem>(*this,
		m_compPhysics, m_compCollisions));
	m_gameSystems.emplace_back(std::make_unique<SpriteSystem>(*this,
		m_compPhysics, m_compSprites, m_compWeapons));
	m_gameSystems.emplace_back(std::make_unique<RoomCollisionSystem>(*this,
		m_compPhysics, m_compCollisions));
	m_gameSystems.emplace_back(std::make_unique<PlayerSystem>(*this,
		m_compPlayer, m_compPhysics, m_compCollisions));
	m_gameSystems.emplace_back(std::make_unique<CharacterSystem>(*this,
		m_compSprites, m_compWeapons, m_compCollisions, m_compAttacks,
		m_compCharacters));

	m_debugSystem = std::make_unique<DebugSystem>(*this,
		m_compPhysics, m_compCollisions, m_compAttacks);

	// TODO: Initialize effects. Remove this later for more flexible approach.
	std::unordered_map<std::string, SpriteAnimation> effectAnims{
		{EffectType::EVADE_SMOKE, { 0, 9, false, glm::vec2(0.f), {0.05f}}},
		{EffectType::HIT_SPARK, { 9, 5, false, glm::vec2(0.f), {0.05f}}},
	};
	m_effectsTexture = std::make_unique<SpriteSheet>("effects.png", effectAnims, glm::ivec2(32, 32));
}

void EntityManager::update(float deltaTime, bool isDebugMode)
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
	return m_compPhysics[m_playerId].pos;
}

const std::vector<std::pair<AABBSource, AABBSource>> &EntityManager::getCollisions() const
{
	return m_collisions;
}

GameEngine &EntityManager::getGameEngine() const
{
	return m_game;
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
	spr.spriteSheet = m_effectsTexture.get();
	spr.spriteSheet->setAnimation(type, spr);
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
	std::unordered_map<std::string, SpriteAnimation> playerAnims{
		{CharState::IDLE, { 0, 8, true, glm::vec2(0.f), {3.f, 0.07f, 0.07f, 0.07f, 0.07f, 1.f, 0.07f, 0.07f}}},
		{CharState::RUN, { 11, 10, true, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_ASCEND, { 22, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_PEAK, { 26, 6, false, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_DESCEND, { 33, 4, true, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_LAND, { 37, 1, false, glm::vec2(0.f), {0.1f} }},
		{CharState::RUN_START, { 38, 5, false, glm::vec2(0.f), {0.07f} }},
		{CharState::RUN_STOP, { 44, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::ALERT, { 48, 1, false, glm::vec2(0.f), {3.f} }},
		{CharState::ALERT_STOP, { 49, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::TURN, { 51, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::CROUCH, { 55, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::CROUCH_STOP, { 59, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::ATTACK, { 61, 10, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.1f} }},
		{CharState::ATTACK_AIR, { 71, 9, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.08f} }},
		{CharState::ATTACK_CROUCH, { 80, 9, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.08f} }},
		{CharState::EVADE_START, { 89, 2, false, glm::vec2(0.f), {0.05f} }},
		{CharState::EVADE, { 91, 4, true, glm::vec2(0.f), {0.05f} }},
		{CharState::ATTACK2, { 95, 10, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.1f} }},
		{CharState::ATTACK3, { 105, 11, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.2f} }},
		{CharState::SKILL1, { 116, 8, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.2f} }},
		{CharState::ATTACK_EVADE, { 124, 10, false, glm::vec2(0.f), {0.05f} }},
	};
	m_playerTexture = std::make_unique<SpriteSheet>("serah_sheet.png", playerAnims, glm::ivec2(32, 32));

	std::unordered_map<std::string, SpriteAnimation> swordAnims{
		{CharState::ATTACK, { 0, 10, false, glm::vec2(8.f, 8.f) }},
		{CharState::ATTACK_AIR, { 11, 9, false, glm::vec2(4.f, 8.f) }},
		{CharState::ATTACK_CROUCH, { 22, 9, false, glm::vec2(5.f, 0.f) }},
		{CharState::ATTACK_EVADE, { 33, 10, false, glm::vec2(10.f, -3.f) }},
		{CharState::ATTACK2, { 55, 10, false, glm::vec2(5.f, 8.f) }},
		{CharState::ATTACK3, { 66, 11, false, glm::vec2(10.f, 8.f) }},
		{CharState::SKILL1, { 44, 8, false, glm::vec2(0.f, 8.f) }},
	};
	m_swordTexture = std::make_unique<SpriteSheet>("serah_sword.png", swordAnims, glm::ivec2(48, 48));

	m_playerId = createEntity({
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_PLAYER,
		GameComponent::COMPONENT_COLLISION,
		GameComponent::COMPONENT_WEAPON,
		GameComponent::COMPONENT_ATTACK,
		GameComponent::COMPONENT_CHARACTER,
	});

	GameComponent::Physics &phys = m_compPhysics[m_playerId];
	phys.pos = glm::vec3(64.f, 800.f, 0.f);
	phys.speed = glm::vec3(0.f);
	phys.scale = glm::vec2(1.f);

	GameComponent::Sprite &spr = m_compSprites[m_playerId];

	// TODO: replace hard-coded frames.
	spr.spriteSheet = m_playerTexture.get();
	spr.spriteSheet->setAnimation(CharState::IDLE, spr);

	GameComponent::Weapon &wp = m_compWeapons[m_playerId];
	wp.spriteSheet = m_swordTexture.get();

	GameComponent::Collision &col = m_compCollisions[m_playerId];
	col.aabb.halfSize = glm::vec2(8, 10);
	col.aabb.offset = glm::vec2(0, -6);

	GameComponent::Attack &atk = m_compAttacks[m_playerId];
	atk.sourceId = m_playerId;

	m_compPlayer.evadeDuration = 0.2f;

	GameComponent::Character &character = m_compCharacters[m_playerId];
	character.attackPatterns = {
		{CharState::ATTACK, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 3, 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK_AIR, {glm::vec2(22, 17), glm::vec2(6, 4), glm::ivec2(2, 6), -1, 0, glm::vec2(96.f, 64.f)}},
		{CharState::ATTACK_CROUCH, {glm::vec2(22, 17), glm::vec2(7, -3), glm::ivec2(2, 6), -1, 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK2, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 3, 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK3, {glm::vec2(19, 21), glm::vec2(15, 6), glm::ivec2(2, 8), -1, 0, glm::vec2(128.f, 0.f)}},
		{CharState::SKILL1, {glm::vec2(14, 23), glm::vec2(10, 9), glm::ivec2(2, 6), -1, 0, glm::vec2(96.f, 256.f)}},
	};

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

	auto jumpEnterAction{ [&phys, &spr, &character, this]()
	{
		InputManager *input{ m_game.getInputManager()};
		bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
		if (isJumping && m_compPlayer.numRemainingJumps > 0)
		{
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
	} };

	auto evadeUpdateAction{ [&phys, &col]()
	{
		bool isColliding{ col.isColliding() };
		// If in the air, reset vertical speed and disable gravity.
		if (!isColliding || (isColliding && phys.speed.y == 0.f))
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
		InputManager *input{ m_game.getInputManager() };
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
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
		createEffect(EffectType::EVADE_SMOKE, phys.pos, phys.scale,
			255, 255, 255, 180);
	} };

	auto runUpdateAction{ [&phys, &character, this]()
	{
		InputManager *input{ m_game.getInputManager()};
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

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
		InputManager *input{ m_game.getInputManager()};
		bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
		bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
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

	auto skill1EnterAction{ [&phys]()
	{
		phys.speed.x = 0.f;
		phys.speed.y = 0.f;
		phys.hasGravity = false;
	} };

	auto skill1ExitAction{ [&phys]()
	{
		phys.hasGravity = true;
	} };

	auto skill1UpdateAction{ [&spr, &phys, &atk]()
	{
		float frameDuration{ GameComponent::getFrameDuration(spr) };
		if (spr.currentFrame == atk.pattern.frameRange.x && !phys.hasGravity)
		{
			float dir{ phys.scale.x > 0 ? 1.f : -1.f };
			phys.speed.x = dir * 32.f;
			phys.speed.y = 256.f;
			phys.isLockedDirection = true;
			phys.hasGravity = true;
		}
	} };

	states.addState(CharState::IDLE);
	states.addState(CharState::RUN, runUpdateAction);
	states.addState(CharState::JUMP_ASCEND, runUpdateAction, jumpEnterAction);
	states.addState(CharState::JUMP_PEAK, runUpdateAction, dropDownEnterAction);
	states.addState(CharState::JUMP_DESCEND, runUpdateAction);
	states.addState(CharState::JUMP_LAND, []() {}, jumpLandEnterAction);
	states.addState(CharState::RUN_START, runUpdateAction);
	states.addState(CharState::RUN_STOP, enableFriction);
	states.addState(CharState::ALERT);
	states.addState(CharState::ALERT_STOP);
	states.addState(CharState::TURN, runUpdateAction);
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
		return (spr.currentFrameTime >= frameDuration &&
			(!spr.currentAnimation.isLooping && spr.currentFrame == spr.currentAnimation.numSprites - 1));
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
		return (phys.speed.y < 0 && !col.isColliding() &&
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
		return col.isColliding() && phys.speed.y < 0;
	} };
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_LAND, isLanding);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_LAND, isLanding);
	states.addEdge(CharState::ATTACK_AIR, CharState::JUMP_LAND, isLanding);

	// Skill 1.
	auto isSkill1{ [this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isSkill1 { input->isKeyPressed(INPUT_SKILL1) };
		return isSkill1;
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
		InputManager *input{ m_game.getInputManager()};
		bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };
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

	auto isAttackCombo{ [&spr, &atk, this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };
		return (isAttacking && spr.currentFrame > atk.pattern.comboFrame);
	} };
	states.addEdge(CharState::ATTACK, CharState::ATTACK2, isAttackCombo);
	states.addEdge(CharState::ATTACK2, CharState::ATTACK3, isAttackCombo);

	auto isAirAttacking{ [&phys, &col, this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };
		bool isColliding{ col.isColliding() };
		return (isAttacking && (!isColliding || (isColliding && phys.speed.y == 0.f)));
	} };
	states.addEdge(CharState::EVADE, CharState::ATTACK_AIR, isAirAttacking);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK_AIR, isAirAttacking);
	states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking);

	// Jumping.
	auto isJumping{ [this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
		return (isJumping && m_compPlayer.numRemainingJumps > 0);
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
		InputManager *input{ m_game.getInputManager()};
		bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
		bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
		return (isCrouching && isJumping && col.isCollidingGhost &&
			!col.isCollidingFloor && !col.isCollidingSlope);
	} };
	states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isDroppingDown);

	// Evading.
	auto isEvading{ [this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isEvading{ input->isKeyPressed(INPUT_EVADE) };
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
		InputManager *input{ m_game.getInputManager()};
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
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
		InputManager *input{ m_game.getInputManager()};
		bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
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
		InputManager *input{ m_game.getInputManager()};
		bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
		return !isCrouching;
	} };
	states.addEdge(CharState::CROUCH, CharState::CROUCH_STOP, isStopCrouching);

	// Start running.
	auto isStartRunning{ [this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return isRunning;
	} };
	auto isTurning{ [&phys, this]() -> bool
	{
		InputManager *input{ m_game.getInputManager()};
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		// Show turning animation if moving in opposite from the direction 
		// the player is facing.
		return isRunning &&
			((phys.scale.x < 0 && isRunningRight) || (phys.scale.x > 0 && isRunningLeft));
	} };
	states.addEdge(CharState::IDLE, CharState::TURN, isTurning);
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
		InputManager *input{ m_game.getInputManager()};
		bool isStopRunningLeft{ input->isKeyReleased(INPUT_LEFT) };
		bool isStopRunningRight{ input->isKeyReleased(INPUT_RIGHT) };
		bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return (isStopRunningLeft || isStopRunningRight || !isRunning);
	} };
	states.addEdge(CharState::RUN, CharState::RUN_STOP, isStopRunning);
	states.addEdge(CharState::RUN_START, CharState::RUN_STOP, isStopRunning);
	states.addEdge(CharState::TURN, CharState::RUN_STOP, isStopRunning);
}

void EntityManager::createEnemy()
{
	std::unordered_map<std::string, SpriteAnimation> enemyAnims{
		{CharState::IDLE, { 0, 1, false, glm::vec2(0.f),  {1.f}}},
		{CharState::RUN, { 4, 4, true, glm::vec2(0.f), {0.07f} }},
		{CharState::ALERT, { 8, 1, false, glm::vec2(0.f), {1.f} }},
		{CharState::HURT, { 12, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::HURT_AIR, { 16, 3, false, glm::vec2(0.f), {0.07f} }},
		{CharState::FALLEN, { 20, 1, false, glm::vec2(0.f), {1.f} }},
		{CharState::ATTACK, { 24, 4, false, glm::vec2(0.f), {0.05f} }},
	};
	m_enemyTexture = std::make_unique<SpriteSheet>("clamper_sheet.png", enemyAnims, glm::ivec2(32, 32));

	int enemyId = createEntity({
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_COLLISION,
		GameComponent::COMPONENT_CHARACTER,
		});

	GameComponent::Physics &phys = m_compPhysics[enemyId];
	phys.pos = glm::vec3(128.f, 800.f, 0.f);
	phys.speed = glm::vec3(0.f);
	phys.scale = glm::vec2(1.f);

	GameComponent::Sprite &spr = m_compSprites[enemyId];

	// TODO: replace hard-coded frames.
	spr.spriteSheet = m_enemyTexture.get();
	spr.spriteSheet->setAnimation(CharState::IDLE, spr);

	GameComponent::Collision &col = m_compCollisions[enemyId];
	col.aabb.halfSize = glm::vec2(8, 10);
	col.aabb.offset = glm::vec2(0, -6);

	// Set up state machine.
	GameComponent::Character &character{ m_compCharacters[enemyId] };
	StateMachine &states{ character.states };

	auto fallenEnterAction{ [&character]()
	{
		character.fallenTimer = 3.f;
	} };

	states.addState(CharState::IDLE);
	states.addState(CharState::HURT);
	states.addState(CharState::HURT_AIR);
	states.addState(CharState::FALLEN, []() {}, fallenEnterAction);

	// Hurt while on ground.
	auto isHurting{ [&character, &col, &phys]() -> bool
	{
		return (character.hitStunTimer > 0.f && col.isColliding() && phys.speed.y < 0);
	} };
	states.addEdge(CharState::IDLE, CharState::HURT, isHurting);

	// Stop hurting.
	auto isStopHurting{ [&character, &col]() -> bool
	{
		return character.hitStunTimer == 0.f;
	} };
	states.addEdge(CharState::HURT, CharState::IDLE, isStopHurting);

	// Hurt while in the air.
	auto isHurtingAir{ [&character, &col, &phys]() -> bool
	{
		bool isColliding{ col.isColliding() };
		return (character.hitStunTimer > 0.f &&
			(!isColliding || (isColliding && phys.speed.y == 0.f)));
	} };
	states.addEdge(CharState::IDLE, CharState::HURT_AIR, isHurtingAir);
	states.addEdge(CharState::HURT, CharState::HURT_AIR, isHurtingAir);

	// Fallen.
	auto isFallen{ [&character, &col, &phys]() -> bool
	{
		return (col.isColliding() && phys.speed.y < 0.f);
	} };
	states.addEdge(CharState::HURT_AIR, CharState::FALLEN, isFallen);

	// Stop fallen.
	auto isStopFallen{ [&character, &col]() -> bool
	{
		return character.fallenTimer == 0.f;
	} };
	states.addEdge(CharState::FALLEN, CharState::IDLE, isStopFallen);
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