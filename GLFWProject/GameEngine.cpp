#include "GameEngine.h"
#include "Renderer.h"

#include <iostream>

namespace
{
	// Set the viewport to determine the size of the rendering window.
	void framebufferSizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
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
	m_window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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
		// Handle user inputs.
		processInput();

		// TODO: change fixed delta.
		// Update values.
		update(0.02f);

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
}

void GameEngine::update(float deltaTime)
{

}

void GameEngine::render(Renderer *renderer)
{
	// Call the renderer.
	renderer->render();

	// Swap the buffers to show the rendered visuals.
	glfwSwapBuffers(m_window);
}