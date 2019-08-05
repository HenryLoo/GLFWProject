#include "GameEngine.h"

#include "Camera.h"
#include "GameSystem.h"
#include "Room.h"
#include "EntityConstants.h"
#include "EntityManager.h"
#include "SpriteSheet.h"

#include "DiskStream.h"
#include "TextureLoader.h"
#include "SpriteSheetLoader.h"
#include "RoomLoader.h"
#include "ShaderLoader.h"

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

	// Initialize the asset loader.
	m_assetLoader = std::make_unique<AssetLoader>(new DiskStream());
	m_assetLoader->registerLoader<Texture>(new TextureLoader());
	m_assetLoader->registerLoader<SpriteSheet>(new SpriteSheetLoader());
	m_assetLoader->registerLoader<Room>(new RoomLoader());
	m_assetLoader->registerLoader<Shader>(new ShaderLoader());

	// Initialize the camera.
	m_camera = std::make_unique<Camera>();

	// Set the callback function for listening to mouse inputs.
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPosCallback(m_window, mouseCallback);
	//glfwSetWindowUserPointer(m_window, this);

	m_sRenderer = std::make_unique<SpriteRenderer>(*this);
	m_uRenderer = std::make_unique<UIRenderer>(*this);
	m_input = std::make_unique<InputManager>();
	m_entityManager = std::make_unique<EntityManager>(*this);

	// TODO: remove this later for more flexible approach.
	m_currentRoom = m_assetLoader->load<Room>("test");
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
	if (m_input->isKeyPressed(InputManager::INPUT_CANCEL))
		glfwSetWindowShouldClose(m_window, true);

	// Toggle debug mode.
	if (m_input->isKeyPressed(InputManager::INPUT_DEBUG))
		m_isDebugMode = !m_isDebugMode;
}

void GameEngine::update()
{
	glm::vec3 playerPos{ m_entityManager->getPlayerPos() };
	if (m_currentRoom != nullptr)
	{
		m_camera->update(m_deltaTime, playerPos, m_windowSize,
			m_currentRoom->getSize());
	}

	m_sRenderer->resetNumSprites();
	m_sRenderer->update(m_camera->getViewMatrix());

	m_uRenderer->resetNumBoxes();

	// Update all entities.
	m_entityManager->update(m_deltaTime, m_isDebugMode);
}

void GameEngine::render()
{
	// Call the renderer.
	m_sRenderer->render(m_windowSize, m_currentRoom.get());

	// Draw hit boxes if debug modes is on.
	if (m_isDebugMode)
		m_uRenderer->render(m_camera.get(), m_windowSize);

	// Swap the buffers to show the rendered visuals.
	glfwSwapBuffers(m_window);
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