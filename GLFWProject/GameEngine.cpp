#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "GameSystem.h"
#include "InputManager.h"
#include "Room.h"
#include "UIRenderer.h"
#include "CharStates.h"

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

	// Initialize the camera.
	m_camera = std::make_unique<Camera>();

	// Set the callback function for listening to mouse inputs.
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPosCallback(m_window, mouseCallback);
	//glfwSetWindowUserPointer(m_window, this);

	// TODO: replace these hardcoded resources.
	createEnemy();
	createPlayer();

	m_currentRoom = std::make_unique<Room>("test");

	m_broadPhase = std::make_unique<CollisionBroadPhase>();
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start(SpriteRenderer *sRenderer, InputManager *input, UIRenderer *uRenderer)
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
		processInput(input);

		// Update values.
		update(sRenderer, input, uRenderer);

		// Call rendering functions.
		render(sRenderer, uRenderer);
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

void GameEngine::processInput(InputManager *input)
{
	input->processInput(m_window);

	// Exit game.
	if (input->isKeyPressed(INPUT_CANCEL))
		glfwSetWindowShouldClose(m_window, true);

	// Toggle debug mode.
	if (input->isKeyPressed(INPUT_DEBUG))
		m_isDebugMode = !m_isDebugMode;
}

void GameEngine::update(SpriteRenderer *sRenderer, InputManager *input, UIRenderer *uRenderer)
{
	m_camera->update(m_deltaTime, m_compPhysics[m_playerId].pos, m_windowSize, 
		m_currentRoom->getSize());

	//createNewEntities();

	sRenderer->resetNumSprites();
	uRenderer->resetNumBoxes();

	// Update all entities.
	glm::vec3 cameraPos{ m_camera->getPosition() };
	GameComponent::Player &player{ m_compPlayer };
	//for (int i = 0; i < m_numEntities; i++)
	//{
	//	unsigned long &e{ m_entities[i] };
	//	GameComponent::Physics &phys{ m_compPhysics[i] };
	//	GameComponent::Sprite &spr{ m_compSprites[i] };
	//	GameComponent::Collision &col{ m_compCollisions[i] };
	//	GameComponent::Weapon &wpn{ m_compWeapons[i] };
	//	GameComponent::Attack &atk{ m_compAttacks[i] };
	//	GameComponent::Enemy &en{ m_compEnemies[i] };
	//	bool isAlive{ true };

	//	// Update relevant components for this entity.
	//	bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
	//	bool hasSprite{ GameComponent::hasComponent(e, GameComponent::COMPONENT_SPRITE) };
	//	bool hasPlayer{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PLAYER) };
	//	bool hasAABB{ GameComponent::hasComponent(e, GameComponent::COMPONENT_COLLISION) };
	//	bool hasWeapon{ GameComponent::hasComponent(e, GameComponent::COMPONENT_WEAPON) };
	//	bool hasAttack{ GameComponent::hasComponent(e, GameComponent::COMPONENT_ATTACK) };
	//	bool hasEnemy{ GameComponent::hasComponent(e, GameComponent::COMPONENT_ENEMY) };
	//	if (isAlive && hasPhysics)
	//	{
	//		isAlive = isAlive && GameSystem::updatePhysics(m_deltaTime, phys);
	//	}
	//	if (isAlive && hasPhysics && hasAABB)
	//	{
	//		isAlive = isAlive && GameSystem::updateRoomCollision(m_deltaTime, 
	//			phys, col, m_currentRoom.get());

	//		// Draw hit boxes if debug mode is on.
	//		if (m_isDebugMode)
	//			uRenderer->addBox(phys, col.aabb, 0, 255, 0, 100);
	//	}
	//	if (isAlive && hasPhysics && hasAttack)
	//	{
	//		// Draw attack hit boxes if debug mode is on.
	//		if (m_isDebugMode && atk.isEnabled)
	//		{
	//			uRenderer->addBox(phys, atk.pattern.aabb, 0, 0, 255, 100);
	//		}
	//	}
	//	if (isAlive && hasPhysics && hasSprite)
	//	{
	//		isAlive = isAlive && GameSystem::updateSprite(m_deltaTime, sRenderer, cameraPos, spr, phys);

	//		if (isAlive && hasWeapon)
	//		{
	//			isAlive = isAlive && GameSystem::updateWeapon(m_deltaTime, sRenderer, cameraPos, spr, phys, wpn);
	//		}
	//	}
	//	if (isAlive && hasPlayer && hasPhysics && hasSprite && hasAABB)
	//	{
	//		isAlive = isAlive && GameSystem::updatePlayer(m_deltaTime, input, player, phys, spr, wpn, col, atk);
	//	}
	//	if (isAlive && hasSprite && hasAttack)
	//	{
	//		isAlive = isAlive && GameSystem::updateAttack(m_deltaTime, spr, atk, phys,
	//			m_playerId, m_enemyIds, m_compPhysics, m_compCollisions);
	//	}

	//	// Flag the entity for deletion if it isn't alive anymore.
	//	if (!isAlive)
	//	{
	//		deleteEntity(i);
	//	}
	//}

	//sRenderer->updateData();


	m_broadPhase->updateAABBList(m_numEntities, m_entities, m_compCollisions, 
		m_compAttacks, m_compPhysics);

	std::vector<std::pair<int, int>> collisions;
	m_broadPhase->generateOverlapList(collisions);
	//std::cout << collisions.size() << std::endl;

	GameSystem::updatePhysics(m_deltaTime, m_numEntities, m_entities, 
		m_compPhysics);
	GameSystem::updateSprite(m_deltaTime, m_numEntities, m_entities, 
		sRenderer, cameraPos, m_compSprites, m_compPhysics);
	GameSystem::updateWeapon(m_deltaTime, m_numEntities, m_entities, 
		sRenderer, cameraPos, m_compSprites, m_compPhysics, m_compWeapons);
	GameSystem::updateRoomCollision(m_deltaTime, m_numEntities, m_entities, 
		m_compPhysics, m_compCollisions, m_currentRoom.get());
	GameSystem::updatePlayer(m_deltaTime, m_entities[m_playerId], 
		input, player, 
		m_compPhysics[m_playerId], m_compSprites[m_playerId], 
		m_compWeapons[m_playerId], m_compCollisions[m_playerId], 
		m_compAttacks[m_playerId]);
	if (m_isDebugMode)
	{
		GameSystem::updateDebug(m_numEntities, m_entities, m_compPhysics, 
			m_compCollisions, m_compAttacks, uRenderer);
	}

	deleteFlaggedEntities();
}

void GameEngine::render(SpriteRenderer *sRenderer, UIRenderer *uRenderer)
{
	// Call the renderer.
	sRenderer->render(m_camera.get(), m_windowSize, m_currentRoom.get());

	// Draw hit boxes if debug modes is on.
	if (m_isDebugMode)
		uRenderer->render(m_camera.get(), m_windowSize);

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

			// Remove this entity's id from the list of all enemy ids.
			m_enemyIds.erase(std::remove(m_enemyIds.begin(), m_enemyIds.end(), id), m_enemyIds.end());
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
		{PlayerState::IDLE, { 0, 8, true, glm::vec2(0.f),  {3.f, 0.07f, 0.07f, 0.07f, 0.07f, 1.f, 0.07f, 0.07f}}},
		{PlayerState::RUN, { 11, 10, true, glm::vec2(0.f), {0.07f} }},
		{PlayerState::JUMP_ASCEND, { 22, 4, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::JUMP_PEAK, { 26, 6, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::JUMP_DESCEND, { 33, 4, true, glm::vec2(0.f), {0.07f} }},
		{PlayerState::JUMP_LAND, { 37, 1, false, glm::vec2(0.f), {0.1f} }},
		{PlayerState::RUN_START, { 38, 5, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::RUN_STOP, { 44, 4, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::ALERT, { 48, 3, false, glm::vec2(0.f), {3.f, 0.07f, 0.07f} }},
		{PlayerState::TURN, { 51, 4, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::CROUCH, { 55, 4, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::CROUCH_STOP, { 59, 2, false, glm::vec2(0.f), {0.07f} }},
		{PlayerState::ATTACK, { 61, 10, false, glm::vec2(0.f), {0.05f} }},
		{PlayerState::ATTACK_AIR, { 71, 9, false, glm::vec2(0.f), {0.05f} }},
		{PlayerState::ATTACK_CROUCH, { 80, 9, false, glm::vec2(0.f), {0.05f} }},
	};
	m_playerTexture = std::make_unique<SpriteSheet>("serah_sheet.png", playerAnims, glm::ivec2(32, 32));

	std::unordered_map<std::string, SpriteAnimation> swordAnims{
		{PlayerState::ATTACK, { 0, 10, false, glm::vec2(8.f, 8.f), {0.05f} }},
		{PlayerState::ATTACK_AIR, { 11, 9, false, glm::vec2(4.f, 8.f), {0.05f} }},
		{PlayerState::ATTACK_CROUCH, { 22, 9, false, glm::vec2(5.f, 0.f), {0.05f} }},
	};
	m_swordTexture = std::make_unique<SpriteSheet>("serah_sword.png", swordAnims, glm::ivec2(48, 48));

	m_playerId = createEntity({
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_PLAYER,
		GameComponent::COMPONENT_COLLISION,
		GameComponent::COMPONENT_WEAPON,
		GameComponent::COMPONENT_ATTACK,
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
	spr.spriteSheet->setAnimation(PlayerState::IDLE, spr);

	GameComponent::Weapon &wp = m_compWeapons[m_playerId];
	wp.spriteSheet = m_swordTexture.get();

	GameComponent::Collision &col = m_compCollisions[m_playerId];
	col.aabb.halfSize = glm::vec2(8, 10);
	col.aabb.offset = glm::vec2(0, -6);

	GameComponent::Attack &atk = m_compAttacks[m_playerId];
	atk.source = m_playerId;

	m_compPlayer.attackPatterns = {
		{PlayerState::ATTACK, {glm::vec2(18, 21), glm::vec2(14, 6), glm::ivec2(2, 7), 0}},
		{PlayerState::ATTACK_AIR, {glm::vec2(22, 17), glm::vec2(6, 4), glm::ivec2(1, 6), 0}},
		{PlayerState::ATTACK_CROUCH, {glm::vec2(22, 17), glm::vec2(7, -3), glm::ivec2(1, 6), 0}}
	};
}

void GameEngine::createEnemy()
{
	std::unordered_map<std::string, SpriteAnimation> enemyAnims{
		{EnemyState::IDLE, { 0, 1, false, glm::vec2(0.f),  {1.f}}},
		{EnemyState::RUN, { 4, 4, true, glm::vec2(0.f), {0.07f} }},
		{EnemyState::ALERT, { 8, 1, false, glm::vec2(0.f), {1.f} }},
		{EnemyState::HURT, { 12, 2, false, glm::vec2(0.f), {0.07f} }},
		{EnemyState::HURT_AIR, { 16, 3, false, glm::vec2(0.f), {0.07f} }},
		{EnemyState::FALLEN, { 20, 1, false, glm::vec2(0.f), {1.f} }},
		{EnemyState::ATTACK, { 24, 4, false, glm::vec2(0.f), {0.05f} }},
	};
	m_enemyTexture = std::make_unique<SpriteSheet>("clamper_sheet.png", enemyAnims, glm::ivec2(32, 32));

	int enemyId = createEntity({
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_COLLISION,
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
	spr.spriteSheet->setAnimation(EnemyState::IDLE, spr);

	GameComponent::Collision &col = m_compCollisions[enemyId];
	col.aabb.halfSize = glm::vec2(8, 10);
	col.aabb.offset = glm::vec2(0, -6);

	// Add this enemy's id to the list of all enemy ids.
	m_enemyIds.push_back(enemyId);
}