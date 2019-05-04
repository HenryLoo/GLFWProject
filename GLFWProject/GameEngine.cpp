#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "GameSystem.h"

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

	void mouseCallback(GLFWwindow *window, double xpos, double ypos)
	{
		GameEngine *game{ (GameEngine *)glfwGetWindowUserPointer(window) };
		game->updateCameraLook(glm::vec2(xpos, ypos));
	}

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
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(m_window, mouseCallback);
	glfwSetWindowUserPointer(m_window, this);
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start(SpriteRenderer *renderer)
{
	// The game loop.
	while (!glfwWindowShouldClose(m_window))
	{
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
		processInput();

		// Update values.
		update(renderer);

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

void GameEngine::processInput()
{
	// Poll IO events.
	glfwPollEvents();

	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	// Move the camera.
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera->move(Camera::Direction::Forward);
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera->move(Camera::Direction::Backward);
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera->move(Camera::Direction::Left);
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera->move(Camera::Direction::Right);

}

void GameEngine::update(SpriteRenderer *renderer)
{
	m_camera->update(m_deltaTime);

	createNewEntities();

	renderer->resetNumSprites();

	// Update all entities.
	glm::vec3 cameraPos{ m_camera->getPosition() };
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		unsigned long &e{ m_entities[i] };
		GameComponent::Physics &phys{ m_compPhysics[i] };
		GameComponent::Sprite &spr{ m_compSprites[i] };

		// Update relevant components for this entity.
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasSprite{ GameComponent::hasComponent(e, GameComponent::COMPONENT_SPRITE) };
		if (hasPhysics)
		{
			GameSystem::updatePhysics(m_deltaTime, phys);
		}
		if (hasPhysics && hasSprite)
		{
			GameSystem::updateSprite(m_deltaTime, renderer, cameraPos, e, spr, phys);
		}
	}

	renderer->updateData();
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

int GameEngine::findUnusedEntity() {

	for (int i = m_lastUsedEntity; i < MAX_ENTITIES; i++)
	{
		if (m_entities[i] == 0)
		{
			m_lastUsedEntity = i;
			return i;
		}
	}

	for (int i = 0; i < m_lastUsedEntity; i++)
	{
		if (m_entities[i] == 0)
		{
			m_lastUsedEntity = i;
			return i;
		}
	}

	// No available entity, so just overwrite the first one.
	return 0;
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
		int entityIndex{ findUnusedEntity() };
		m_compSprites[entityIndex].duration = 5.0f;
		m_compPhysics[entityIndex].pos = glm::vec3(0, 0, -20.0f);

		float spread{ 1.5f };
		glm::vec3 mainDir{ glm::vec3(0.0f, 10.0f, 0.0f) };
		glm::vec3 randomDir{ glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		) };

		m_compPhysics[entityIndex].speed = mainDir + randomDir * spread;

		m_compSprites[entityIndex].r = rand() % 256;
		m_compSprites[entityIndex].g = rand() % 256;
		m_compSprites[entityIndex].b = rand() % 256;
		m_compSprites[entityIndex].a = 255;

		m_compPhysics[entityIndex].scale = (rand() % 1000) / 2000.0f + 0.1f;

		GameComponent::addComponent(m_entities[entityIndex], 
			GameComponent::ComponentType::COMPONENT_PHYSICS);
		GameComponent::addComponent(m_entities[entityIndex],
			GameComponent::ComponentType::COMPONENT_SPRITE);
	}
}