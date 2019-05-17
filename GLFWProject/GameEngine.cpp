#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "GameSystem.h"
#include "InputManager.h"
#include "Room.h"

#include <algorithm>
#include <iostream>

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
	m_window = glfwCreateWindow(1024, 768, "GLFWProject", NULL, NULL);
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
	std::unordered_map<std::string, SpriteAnimation> anims{
		{"idle", { 0, 8, {3.f, 0.07f, 0.07f, 0.07f, 0.07f, 1.f, 0.07f, 0.07f}}},
		{"run", { 10, 10, {0.07f, 0.07f, 0.07f, 0.07f, 0.07f, 0.07f, 0.07f, 0.07f, 0.07f, 0.07f} }}
	};
	m_texture = std::make_unique<SpriteSheet>("serah_sheet.png", anims, glm::vec2(32, 32));
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start(SpriteRenderer *renderer, InputManager *input)
{
	createPlayer();

	// TODO: replace this hard-coded room later.
	glm::ivec2 size{ 10, 10 };
	std::vector<TileType> tileTypes{
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE, TILE_SPACE,
		TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL,
	};
	std::vector<unsigned int> tileSprites{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	std::unique_ptr<Room> room{ std::make_unique<Room>(size, tileTypes, tileSprites) };

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
			int width, height;
			glfwGetWindowSize(m_window, &width, &height);
			//renderer->createFramebuffer(width, height);
		}

		float currentFrame{ static_cast<float>(glfwGetTime()) };
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		// Handle user inputs.
		input->processInput(m_window);
		if (input->isKeyPressed(INPUT_CANCEL))
			glfwSetWindowShouldClose(m_window, true);

		// Update values.
		update(renderer, input);

		// Call rendering functions.
		render(renderer);
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

//void GameEngine::processInput()
//{
//	// Poll IO events.
//	glfwPollEvents();
//
//	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(m_window, true);
//
//	// Move the camera.
//	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
//		m_camera->move(Camera::Direction::Forward);
//	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
//		m_camera->move(Camera::Direction::Backward);
//	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
//		m_camera->move(Camera::Direction::Left);
//	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
//		m_camera->move(Camera::Direction::Right);
//
//}

void GameEngine::update(SpriteRenderer *renderer, InputManager *input)
{
	m_camera->update(m_deltaTime);

	//createNewEntities();

	renderer->resetNumSprites();

	// Update all entities.
	glm::vec3 cameraPos{ m_camera->getPosition() };
	GameComponent::Player &player{ m_compPlayer };
	for (int i = 0; i < m_numEntities; i++)
	{
		unsigned long &e{ m_entities[i] };
		GameComponent::Physics &phys{ m_compPhysics[i] };
		GameComponent::Sprite &spr{ m_compSprites[i] };
		bool isAlive{ true };

		// Update relevant components for this entity.
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasSprite{ GameComponent::hasComponent(e, GameComponent::COMPONENT_SPRITE) };
		bool hasPlayer{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PLAYER) };
		if (isAlive && hasPhysics)
		{
			isAlive = isAlive && GameSystem::updatePhysics(m_deltaTime, phys);
		}
		if (isAlive && hasPhysics && hasSprite)
		{
			isAlive = isAlive && GameSystem::updateSprite(m_deltaTime, renderer, cameraPos, spr, phys);
		}
		if (isAlive && hasPlayer && hasPhysics && hasSprite)
		{
			isAlive = isAlive && GameSystem::updatePlayer(input, player, phys, spr);
		}

		// Flag the entity for deletion if it isn't alive anymore.
		if (!isAlive)
		{
			deleteEntity(i);
		}
	}

	renderer->updateData();

	deleteFlaggedEntities();
}

void GameEngine::render(SpriteRenderer *renderer)
{
	// Call the renderer.
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
	renderer->render(m_camera.get(), width / (float) height);

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

		spr.isLooping = true;
		spr.spriteSheet = m_texture.get();
		spr.spriteSheet->setAnimation("run", spr);
		//spr.frames = {
		//	{20, 0.07f},
		//	{21, 0.07f},
		//	{22, 0.07f},
		//	{23, 0.07f},
		//	{24, 0.07f},
		//	{25, 0.07f},
		//	{26, 0.07f},
		//	{27, 0.07f},
		//	{28, 0.07f},
		//	{29, 0.07f},
		//};
	}
}

void GameEngine::createPlayer()
{
	int entityIndex{ createEntity({
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_PLAYER,
	}) };

	GameComponent::Physics &phys = m_compPhysics[entityIndex];
	phys.pos = glm::vec3(0.f, 0.f, 0.f);
	phys.speed = glm::vec3(0.f);
	phys.scale = glm::vec2(1.f);

	GameComponent::Sprite &spr = m_compSprites[entityIndex];
	spr.r = 255;
	spr.g = 255;
	spr.b = 255;
	spr.a = 255;
	spr.hasDuration = false;
	spr.isLooping = true;

	// TODO: replace hard-coded frames.
	spr.spriteSheet = m_texture.get();
	spr.spriteSheet->setAnimation("idle", spr);
	//spr.frames = {
	//	{20, 0.07f},
	//	{21, 0.07f},
	//	{22, 0.07f},
	//	{23, 0.07f},
	//	{24, 0.07f},
	//	{25, 0.07f},
	//	{26, 0.07f},
	//	{27, 0.07f},
	//	{28, 0.07f},
	//	{29, 0.07f},
	//};
}