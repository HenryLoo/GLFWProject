#include "GameEngine.h"

#include "Camera.h"
#include "GameSystem.h"
#include "Room.h"
#include "EntityConstants.h"
#include "AssetLoader.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "SpriteRenderer.h"
#include "UIRenderer.h"
#include "TextRenderer.h"
#include "Font.h"
#include "PlayState.h"

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

	// Set the callback function for listening to mouse inputs.
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPosCallback(m_window, mouseCallback);
	//glfwSetWindowUserPointer(m_window, this);
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start(EntityManager *entityManager, AssetLoader *assetLoader,
	InputManager *inputManager, SpriteRenderer *sRenderer,
	UIRenderer *uRenderer, TextRenderer *tRenderer)
{
	double previousTime = glfwGetTime();
	int frameCount = 0;

	m_debugFont = assetLoader->load<Font>("default", 16);

	// The game loop.
	while (!glfwWindowShouldClose(m_window))
	{
		double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - previousTime >= 1.0)
		{
			m_fps = frameCount;
			frameCount = 0;
			previousTime = currentTime;
		}

		// If the window size was changed, update the renderer.
		if (m_hasNewWindowSize)
		{
			m_hasNewWindowSize = false;
			glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
			sRenderer->createFramebuffer(m_windowSize);
			Renderer::updateWindowSize(m_windowSize);
		}

		float currentFrame{ static_cast<float>(glfwGetTime()) };
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		// Handle user inputs.
		processInput(inputManager, assetLoader);

		// Update values.
		update(entityManager, assetLoader, sRenderer, uRenderer, tRenderer);

		// Call rendering functions.
		render(sRenderer, uRenderer, tRenderer);
	}
}

void GameEngine::updateRendererSize()
{
	m_hasNewWindowSize = true;
}

void GameEngine::changeState(GameState *state, AssetLoader *assetLoader)
{
	// Clean up the current state and pop it.
	if (!m_states.empty())
	{
		m_states.back()->cleanUp();
		m_states.pop_back();
	}

	// Push the new state onto the stack.
	m_states.push_back(state);
	m_states.back()->init(assetLoader);
}

void GameEngine::pushState(GameState *state, AssetLoader *assetLoader)
{
	// Pause the current state.
	if (!m_states.empty())
	{
		m_states.back()->pause();
	}

	// Push the new state onto the stack.
	m_states.push_back(state);
	m_states.back()->init(assetLoader);
}

void GameEngine::popState()
{
	// Clean up the current state and pop it.
	if (!m_states.empty())
	{
		m_states.back()->cleanUp();
		m_states.pop_back();
	}

	// Resume the previous state.
	if (!m_states.empty())
	{
		m_states.back()->resume();
	}
}

void GameEngine::processInput(InputManager *inputManager, AssetLoader *assetLoader)
{
	glfwPollEvents();

	inputManager->update(m_deltaTime);

	// Toggle debug mode.
	if (inputManager->isKeyPressed(InputManager::INPUT_DEBUG, true))
		m_isDebugMode = !m_isDebugMode;

	// Process inputs for the current game state.
	if (!m_states.empty())
	{
		m_states.back()->processInput(this, inputManager, assetLoader);
	}
}

void GameEngine::update(EntityManager *entityManager, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer, TextRenderer *tRenderer)
{
	assetLoader->update(m_deltaTime);

	// Update the current game state.
	if (!m_states.empty())
	{
		m_states.back()->update(m_deltaTime, m_windowSize,
			entityManager, assetLoader, sRenderer, uRenderer, tRenderer);
	}

	if (m_isDebugMode)
	{
		entityManager->updateDebug(m_deltaTime);
		tRenderer->addText("FPS: " + std::to_string(m_fps), m_debugFont.get(),
			glm::vec2(16.f, m_windowSize.y - 32.f));
	}
}

void GameEngine::render(SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer)
{
	// Clear the screen.
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the current game state.
	if (!m_states.empty())
	{
		m_states.back()->render(m_windowSize, sRenderer, uRenderer, tRenderer);
	}

	// Draw hit boxes if debug modes is on.
	if (m_isDebugMode)
	{
		uRenderer->renderBoxes(PlayState::instance()->getCamera());
	}

	// Swap the buffers to show the rendered visuals.
	glfwSwapBuffers(m_window);
}

GLFWwindow *GameEngine::getWindow() const
{
	return m_window;
}

void GameEngine::quit() const
{
	glfwSetWindowShouldClose(m_window, true);
}