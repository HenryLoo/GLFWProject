#include "GameEngine.h"
#include "Renderer.h"
#include "Camera.h"

#include <iostream>

namespace
{
	// Set the viewport to determine the size of the rendering window.
	void framebufferSizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void mouseCallback(GLFWwindow *window, double xpos, double ypos)
	{
		Camera *camera = (Camera *)glfwGetWindowUserPointer(window);
		camera->lookAt(glm::vec2(xpos, ypos));
	}
}

GameEngine::GameEngine()
{
	// Initialize GLFW with OpenGL 3.3+.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Uncomment for Mac OS X.
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the window
	m_window = glfwCreateWindow(800, 600, "GLFWProject", NULL, NULL);
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
	glfwSetWindowUserPointer(m_window, m_camera.get());
}

GameEngine::~GameEngine()
{
	// Deallocate all resources and terminate GLFW.
	glfwTerminate();
}

void GameEngine::start(Renderer *renderer)
{
	// The game loop.
	while (!glfwWindowShouldClose(m_window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		// Handle user inputs.
		processInput();

		// TODO: change fixed delta.
		// Update values.
		update();

		// Call rendering functions.
		render(renderer);
	}
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

void GameEngine::update()
{
	m_camera->update(m_deltaTime);
}

void GameEngine::render(Renderer *renderer)
{
	// Call the renderer.
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
	renderer->render(m_camera.get(), width / (float) height);

	// Swap the buffers to show the rendered visuals.
	glfwSwapBuffers(m_window);
}