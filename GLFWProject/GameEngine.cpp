#include "GameEngine.h"

#include "Camera.h"
#include "GameSystem.h"
#include "Room.h"
#include "CharStates.h"
#include "EntityConstants.h"
#include "PhysicsSystem.h"
#include "SpriteSystem.h"
#include "RoomCollisionSystem.h"
#include "PlayerSystem.h"
#include "AttackSystem.h"
#include "AttackCollisionSystem.h"
#include "CharacterSystem.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace
{
	// Set the viewport to determine the size of the rendering window.
	void framebufferSizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);

		GameEngine *game{ (GameEngine *)glfwGetWindowUserPointer(window) };
		game->updateRendererSize();
	}

	//void mouseCallback(GLFWwindow *window, double xpos, double ypos)
	//{
	//	GameEngine *game{ (GameEngine *)glfwGetWindowUserPointer(window) };
	//	game->updateCameraLook(glm::vec2(xpos, ypos));
	//}

	// TODO: remove these later.
	const float SECONDS_PER_FRAME{ 1 / 60.f };
	const int NUM_ENTITIES_PER_SECOND{ 1000 };
}

GameEngine::GameEngine()
{
	// Initialize GLFW with OpenGL 3.3+.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Uncomment for Mac OS X.
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the window
	m_window = glfwCreateWindow(1280, 800, "GLFWProject", NULL, NULL);
	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_window);

	// Initialize GLAD before calling any OpenGL functions.
	// glfwGetProcAddress gets the OS-specific OpenGL function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Set the callback function for automatically setting the viewport
	// when the window is resized.
	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

	// Initialize entity and component vectors.
	m_entities.resize(EntityConstants::MAX_ENTITIES);
	m_compPhysics.resize(EntityConstants::MAX_ENTITIES);
	m_compSprites.resize(EntityConstants::MAX_ENTITIES);
	m_compCollisions.resize(EntityConstants::MAX_ENTITIES);
	m_compWeapons.resize(EntityConstants::MAX_ENTITIES);
	m_compAttacks.resize(EntityConstants::MAX_ENTITIES);
	m_compEnemies.resize(EntityConstants::MAX_ENTITIES);
	m_compCharacters.resize(EntityConstants::MAX_ENTITIES);

	// Initialize the camera.
	m_camera = std::make_unique<Camera>();

	// Set the callback function for listening to mouse inputs.
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPosCallback(m_window, mouseCallback);
	//glfwSetWindowUserPointer(m_window, this);

	m_sRenderer = std::make_unique<SpriteRenderer>();
	m_uRenderer = std::make_unique<UIRenderer>();
	m_input = std::make_unique<InputManager>();

	// TODO: replace these hardcoded resources.
	createEnemy();
	createPlayer();

	m_currentRoom = std::make_unique<Room>("test");

	m_broadPhase = std::make_unique<CollisionBroadPhase>();

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
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start()
{
	double previousTime = glfwGetTime();
	int frameCount = 0;

	// The game loop.
	while (!glfwWindowShouldClose(m_window))
	{
		double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - previousTime >= 1.0)
		{
			std::cout << "FPS: " << frameCount << std::endl;

			frameCount = 0;
			previousTime = currentTime;
		}

		// If the window size was changed, update the renderer.
		if (m_hasNewWindowSize)
		{
			m_hasNewWindowSize = false;
			glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
			//renderer->createFramebuffer(width, height);
		}

		float currentFrame{ static_cast<float>(glfwGetTime()) };
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		// Handle user inputs.
		processInput();

		// Update values.
		update();

		// Call rendering functions.
		render();
	}
}

void GameEngine::updateCameraLook(glm::vec2 screenPos)
{
	m_camera->lookAt(screenPos);
}

void GameEngine::updateRendererSize()
{
	m_hasNewWindowSize = true;
}

void GameEngine::processInput()
{
	m_input->processInput(m_window);

	// Exit game.
	if (m_input->isKeyPressed(INPUT_CANCEL))
		glfwSetWindowShouldClose(m_window, true);

	// Toggle debug mode.
	if (m_input->isKeyPressed(INPUT_DEBUG))
		m_isDebugMode = !m_isDebugMode;
}

void GameEngine::update()
{
	m_camera->update(m_deltaTime, m_compPhysics[m_playerId].pos, m_windowSize, 
		m_currentRoom->getSize());

	//createNewEntities();

	m_sRenderer->resetNumSprites();
	m_uRenderer->resetNumBoxes();

	// Update all entities.
	glm::vec3 cameraPos{ m_camera->getPosition() };

	//sRenderer->updateData();

	// Get all collisions and handle them.
	m_broadPhase->updateAABBList(m_numEntities, m_entities, m_compCollisions, 
		m_compAttacks, m_compPhysics);
	m_broadPhase->generateOverlapList(m_collisions);

	// Process all game systems.
	for (auto &system : m_gameSystems)
	{
		system->update(m_deltaTime, m_numEntities, m_entities);
	}

	// Only process debug system if in debug mode.
	if (m_isDebugMode)
	{
		m_debugSystem->update(m_deltaTime, m_numEntities, m_entities);
	}

	// Reset overlaps list.
	m_collisions.clear();

	deleteFlaggedEntities();
}

void GameEngine::render()
{
	// Call the renderer.
	m_sRenderer->render(m_camera.get(), m_windowSize, m_currentRoom.get());

	// Draw hit boxes if debug modes is on.
	if (m_isDebugMode)
		m_uRenderer->render(m_camera.get(), m_windowSize);

	// Swap the buffers to show the rendered visuals.
	glfwSwapBuffers(m_window);
}

int GameEngine::createEntity(std::vector<GameComponent::ComponentType> types)
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

void GameEngine::deleteEntity(int id)
{
	// Flag the entity to be deleted at the end of the game loop.
	m_entitiesToDelete.push_back(id);
}

SpriteRenderer *GameEngine::getSpriteRenderer() const
{
	return m_sRenderer.get();
}

UIRenderer *GameEngine::getUIRenderer() const
{
	return m_uRenderer.get();
}

InputManager *GameEngine::getInputManager() const
{
	return m_input.get();
}

Camera *GameEngine::getCamera() const
{
	return m_camera.get();
}

Room *GameEngine::getCurrentRoom() const
{
	return m_currentRoom.get();
}

int GameEngine::getPlayerId() const
{
	return m_playerId;
}

const std::vector<std::pair<AABBSource, AABBSource>> &GameEngine::getCollisions() const
{
	return m_collisions;
}

void GameEngine::deleteFlaggedEntities()
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


void GameEngine::createNewEntities()
{
	// The number of new sprites to create this frame.
	int numNewEntities{ (int)(m_deltaTime * NUM_ENTITIES_PER_SECOND) };
	if (numNewEntities > (int)(SECONDS_PER_FRAME * NUM_ENTITIES_PER_SECOND))
		numNewEntities = (int)(SECONDS_PER_FRAME * NUM_ENTITIES_PER_SECOND);

	// Generate the new sprites with random values.
	for (int i = 0; i < numNewEntities; i++)
	{
		int entityIndex{ createEntity({ 
			GameComponent::COMPONENT_PHYSICS,
			GameComponent::COMPONENT_SPRITE,
		}) };

		GameComponent::Physics &phys = m_compPhysics[entityIndex];
		phys.pos = glm::vec3(0, 5, -20.0f);

		float spread{ 1.5f };
		glm::vec3 mainDir{ glm::vec3(0.0f, 10.0f, 0.0f) };
		glm::vec3 randomDir{ glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		) };

		phys.speed = mainDir + randomDir * spread;
		phys.scale = glm::vec2((rand() % 1000) / 2000.0f + 0.1f);

		GameComponent::Sprite &spr = m_compSprites[entityIndex];
		spr.duration = 5.0f;
		spr.r = rand() % 256;
		spr.g = rand() % 256;
		spr.b = rand() % 256;
		spr.a = 255;

		spr.spriteSheet = m_playerTexture.get();
		spr.spriteSheet->setAnimation("run", spr);
	}
}

void GameEngine::createPlayer()
{
	std::unordered_map<std::string, SpriteAnimation> playerAnims{
		{CharState::IDLE, { 0, 8, true, glm::vec2(0.f),  {3.f, 0.07f, 0.07f, 0.07f, 0.07f, 1.f, 0.07f, 0.07f}}},
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
	spr.r = 255;
	spr.g = 255;
	spr.b = 255;
	spr.a = 255;
	spr.hasDuration = false;

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
		{CharState::ATTACK, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK_AIR, {glm::vec2(22, 17), glm::vec2(6, 4), glm::ivec2(2, 6), 0, glm::vec2(96.f, 64.f)}},
		{CharState::ATTACK_CROUCH, {glm::vec2(22, 17), glm::vec2(7, -3), glm::ivec2(2, 6), 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK_EVADE, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK2, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 0, glm::vec2(96.f, 0.f)}},
		{CharState::ATTACK3, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 0, glm::vec2(96.f, 0.f)}},
	};

	// Set up state machine.
	StateMachine &states{ character.states };
	states.addState(CharState::IDLE);
	states.addState(CharState::RUN);
	states.addState(CharState::JUMP_ASCEND);
	states.addState(CharState::JUMP_PEAK);
	states.addState(CharState::JUMP_DESCEND);
	states.addState(CharState::JUMP_LAND);
	states.addState(CharState::RUN_START);
	states.addState(CharState::RUN_STOP);
	states.addState(CharState::ALERT);
	states.addState(CharState::ALERT_STOP);
	states.addState(CharState::TURN);
	states.addState(CharState::CROUCH);
	states.addState(CharState::CROUCH_STOP);
	states.addState(CharState::ATTACK);
	states.addState(CharState::ATTACK_AIR);
	states.addState(CharState::ATTACK_CROUCH);
	states.addState(CharState::ATTACK_EVADE);
	states.addState(CharState::EVADE_START);
	states.addState(CharState::EVADE);
	states.addState(CharState::ATTACK2);
	states.addState(CharState::ATTACK3);
	states.addState(CharState::SKILL1);

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

	// Attacking.
	auto isAttacking{ [this]() -> bool
	{
		bool isAttacking{ m_input->isKeyPressed(INPUT_ATTACK) };
		return isAttacking;
	} };
	auto stopAction{ [&phys]()
	{
		phys.hasFriction = true;
	} };
	states.addEdge(CharState::IDLE, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::ALERT, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::ALERT_STOP, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::RUN_START, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::RUN, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::RUN_STOP, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::TURN, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::CROUCH, CharState::ATTACK_CROUCH, isAttacking);
	states.addEdge(CharState::CROUCH_STOP, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::JUMP_LAND, CharState::ATTACK, isAttacking);
	states.addEdge(CharState::JUMP_ASCEND, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::JUMP_PEAK, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::JUMP_DESCEND, CharState::ATTACK_AIR, isAttacking);
	states.addEdge(CharState::ATTACK, CharState::ATTACK2, isAttacking, stopAction);
	states.addEdge(CharState::ATTACK2, CharState::ATTACK3, isAttacking, stopAction);

	auto isAirAttacking{ [&phys, &col, this]() -> bool
	{
		bool isAttacking{ m_input->isKeyPressed(INPUT_ATTACK) };
		bool isColliding{ col.isColliding() };
		return (isAttacking && (!isColliding || (isColliding && phys.speed.y == 0.f)));
	} };
	auto evadeStopAction{ [&phys, &character, &col]()
	{
		phys.hasGravity = true;
	} };
	states.addEdge(CharState::EVADE, CharState::ATTACK_AIR, isAirAttacking, evadeStopAction);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK_AIR, isAirAttacking, evadeStopAction);
	states.addEdge(CharState::EVADE, CharState::ATTACK, isAttacking, stopAction);
	states.addEdge(CharState::EVADE_START, CharState::ATTACK, isAttacking, stopAction);

	// Attacking air chain.
	//auto attackAirAction{ [&spr]()
	//{
	//	spr.isResetAnimation = true;
	//} };
	//states.addEdge(CharState::ATTACK_AIR, CharState::ATTACK_AIR, isAttacking, attackAirAction);

	// Jumping.
	auto isJumping{ [this]() -> bool
	{
		bool isJumping{ m_input->isKeyPressed(INPUT_JUMP) };
		return (isJumping && m_compPlayer.numRemainingJumps > 0);
	} };
	auto jumpAction{ [&phys, &spr, &col, &character, this]()
	{
		phys.hasGravity = true;
		spr.isResetAnimation = true;
		m_compPlayer.numRemainingJumps--;
		phys.speed.y = character.jumpSpeed;
	} };
	states.addEdge(CharState::IDLE, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::ALERT, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::ALERT_STOP, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::RUN_START, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::RUN, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::RUN_STOP, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::TURN, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::CROUCH_STOP, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::JUMP_LAND, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::ATTACK, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::ATTACK2, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::ATTACK3, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::EVADE_START, CharState::JUMP_ASCEND, isJumping, jumpAction);
	states.addEdge(CharState::EVADE, CharState::JUMP_ASCEND, isJumping, jumpAction);

	// Drop down.
	auto isDroppingDown{ [&col, this]() -> bool
	{
		bool isCrouching{ m_input->isKeyPressing(INPUT_DOWN) };
		bool isJumping{ m_input->isKeyPressed(INPUT_JUMP) };
		return (isCrouching && isJumping && col.isCollidingGhost && 
			!col.isCollidingFloor && !col.isCollidingSlope);
	} };
	auto dropDownAction{ [&phys, &spr, &col, &character, this]()
	{
		spr.isResetAnimation = true;
		m_compPlayer.numRemainingJumps--;
		phys.speed.y = 0.f;
		phys.pos.y--;
	} };
	states.addEdge(CharState::CROUCH, CharState::JUMP_PEAK, isDroppingDown, dropDownAction);

	// Evading.
	auto isEvading{ [this]() -> bool 
	{
		bool isEvading{ m_input->isKeyPressed(INPUT_EVADE) };
		return (isEvading && m_compPlayer.numRemainingEvades > 0);
	} };
	auto evadeAction{ [&phys, &character, this]() 
	{
		float dir{ phys.scale.x > 0 ? 1.f : -1.f };
		m_compPlayer.numRemainingEvades--;
		phys.speed.x = dir * 1.7f * character.movementSpeed;
		phys.hasFriction = false;

		// Set the evade timer.
		m_compPlayer.evadeTimer = m_compPlayer.evadeDuration;
	} };
	states.addEdge(CharState::IDLE, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ALERT, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ALERT_STOP, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::RUN_START, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::RUN, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::RUN_STOP, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::TURN, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::CROUCH_STOP, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::JUMP_LAND, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ATTACK, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ATTACK2, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ATTACK3, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::JUMP_ASCEND, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::JUMP_PEAK, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::JUMP_DESCEND, CharState::EVADE_START, isEvading, evadeAction);
	states.addEdge(CharState::ATTACK_AIR, CharState::EVADE_START, isEvading, evadeAction);

	// Stop evading.
	auto isEvadeTimerEnd{ [this]() -> bool 
	{
		return m_compPlayer.evadeTimer == 0.f;
	} };
	states.addEdge(CharState::EVADE_START, CharState::RUN, isEvadeTimerEnd, evadeStopAction);
	states.addEdge(CharState::EVADE, CharState::RUN, isEvadeTimerEnd, evadeStopAction);

	// Evading in the air.
	auto isEvadingAir{ [&phys, &col]() -> bool 
	{
		bool isColliding{ col.isColliding() };
		return (!isColliding || (isColliding && phys.speed.y == 0.f));
	} };
	auto evadeAirAction{ [&phys, &character]()
	{
		phys.speed.y = 0.f;
		phys.hasGravity = false;
	} };
	states.addEdge(CharState::EVADE_START, CharState::EVADE_START, isEvadingAir, evadeAirAction);
	states.addEdge(CharState::EVADE, CharState::EVADE, isEvadingAir, evadeAirAction);

	// Crouching.
	auto isCrouching{ [this]() -> bool
	{
		bool isCrouching{ m_input->isKeyPressing(INPUT_DOWN) };
		return isCrouching;
	} };
	states.addEdge(CharState::IDLE, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::ALERT, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::ALERT_STOP, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::RUN_START, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::RUN, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::RUN_STOP, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::TURN, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::CROUCH_STOP, CharState::CROUCH, isCrouching, stopAction);
	states.addEdge(CharState::JUMP_LAND, CharState::CROUCH, isCrouching, stopAction);

	// Stop crouching.
	auto isStopCrouching{ [this]() -> bool
	{
		bool isCrouching{ m_input->isKeyPressing(INPUT_DOWN) };
		return !isCrouching;
	} };
	states.addEdge(CharState::CROUCH, CharState::CROUCH_STOP, isStopCrouching);

	// Start running.
	auto isStartRunning{ [this]() -> bool
	{
		bool isRunningLeft{ m_input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ m_input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return isRunning;
	} };
	auto runAction{ [&phys, &character, this]()
	{
		bool isRunningLeft{ m_input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ m_input->isKeyPressing(INPUT_RIGHT) };

		// If current speed is greater than the character's movement speed,
		// start applying friction to gradually bring it back down to
		// the character's movement speed.
		float currentSpeed{ glm::abs(phys.speed.x) };
		phys.hasFriction = (currentSpeed > character.movementSpeed);

		// Maintain maximum horizontal speed.
		float maxSpeed{ glm::max(character.movementSpeed, currentSpeed) };

		float dir{ phys.scale.x };
		if (isRunningLeft)
		{
			dir = -1;
		}
		else if (isRunningRight)
		{
			dir = 1;
		}

		phys.speed.x += (dir * character.movementSpeed / 0.1f * m_deltaTime);
		phys.speed.x = glm::clamp(phys.speed.x, -maxSpeed, maxSpeed);

		if (character.states.getState() != CharState::ATTACK_AIR)
			phys.scale.x = dir;
	} };
	auto isTurning{ [&phys, this]() -> bool
	{
		bool isRunningLeft{ m_input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ m_input->isKeyPressing(INPUT_RIGHT) };
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

	states.addEdge(CharState::RUN_START, CharState::RUN_START, isStartRunning, runAction);
	states.addEdge(CharState::RUN, CharState::RUN, isStartRunning, runAction);
	states.addEdge(CharState::TURN, CharState::TURN, isStartRunning, runAction);
	states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_ASCEND, isStartRunning, runAction);
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_PEAK, isStartRunning, runAction);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_DESCEND, isStartRunning, runAction);
	states.addEdge(CharState::ATTACK_AIR, CharState::ATTACK_AIR, isStartRunning, runAction);

	// Stop running.
	auto isStopRunning{ [&character, this]() -> bool
	{
		bool isStopRunningLeft{ m_input->isKeyReleased(INPUT_LEFT) };
		bool isStopRunningRight{ m_input->isKeyReleased(INPUT_RIGHT) };
		bool isRunningLeft{ m_input->isKeyPressing(INPUT_LEFT) };
		bool isRunningRight{ m_input->isKeyPressing(INPUT_RIGHT) };
		bool isRunning{ isRunningLeft != isRunningRight };

		return (isStopRunningLeft || isStopRunningRight || !isRunning);
	} };
	auto stopRunAction{ [&phys]() 
	{
		phys.hasFriction = true;
	} };
	states.addEdge(CharState::RUN, CharState::RUN_STOP, isStopRunning, stopRunAction);
	states.addEdge(CharState::RUN_START, CharState::RUN_STOP, isStopRunning, stopRunAction);
	states.addEdge(CharState::TURN, CharState::RUN_STOP, isStopRunning, stopRunAction);
	states.addEdge(CharState::JUMP_ASCEND, CharState::JUMP_ASCEND, isStopRunning, stopRunAction);
	states.addEdge(CharState::JUMP_PEAK, CharState::JUMP_PEAK, isStopRunning, stopRunAction);
	states.addEdge(CharState::JUMP_DESCEND, CharState::JUMP_DESCEND, isStopRunning, stopRunAction);
	states.addEdge(CharState::ATTACK_AIR, CharState::ATTACK_AIR, isStopRunning, stopRunAction);
}

void GameEngine::createEnemy()
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
	spr.r = 255;
	spr.g = 255;
	spr.b = 255;
	spr.a = 255;
	spr.hasDuration = false;

	// TODO: replace hard-coded frames.
	spr.spriteSheet = m_enemyTexture.get();
	spr.spriteSheet->setAnimation(CharState::IDLE, spr);

	GameComponent::Collision &col = m_compCollisions[enemyId];
	col.aabb.halfSize = glm::vec2(8, 10);
	col.aabb.offset = glm::vec2(0, -6);
	
	// Set up state machine.
	GameComponent::Character &character{ m_compCharacters[enemyId] };
	StateMachine &states{ character.states };
	states.addState(CharState::IDLE);
	states.addState(CharState::HURT);
	states.addState(CharState::HURT_AIR);
	states.addState(CharState::FALLEN);

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

	// Fallen.
	auto isFallen{ [&character, &col, &phys]() -> bool
	{
		return (col.isColliding() && phys.speed.y < 0.f);
	} };
	auto fallenAction{ [&character]()
	{
		character.fallenTimer = 3.f;
	} };
	states.addEdge(CharState::HURT_AIR, CharState::FALLEN, isFallen, fallenAction);

	// Stop fallen.
	auto isStopFallen{ [&character, &col]() -> bool
	{
		return character.fallenTimer == 0.f;
	} };
	states.addEdge(CharState::FALLEN, CharState::IDLE, isStopFallen);
}