#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "shader.h"
#include "texture.h"

// Forward declarations.
GLFWwindow* init();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void prepareSquare();
void render();
void cleanUp();

unsigned int VBO, VAO, EBO;

int main()
{
	GLFWwindow* window = init();

	// Exit if the window could not be created.
	if (window == nullptr)
		return -1;


	// Load the shader.
	Shader theShader("default.vs", "default.fs");

	// Load the texture.
	Texture theTexture("serah_idle.png");

	// Prepare the triangle's vertices.
	prepareSquare();

	// Render loop.
	while (!glfwWindowShouldClose(window))
	{
		// Handle user inputs.
		processInput(window);

		// Set the shader program.
		theShader.use();

		// Bind to the texture.
		theTexture.bind();

		// Call rendering functions.
		render();

		// Swap the buffers to render.
		glfwSwapBuffers(window);

		// Poll IO events.
		glfwPollEvents();
	}

	// Clean up.
	cleanUp();

	return 0;
}

// Initialize GLFW and GLAD
GLFWwindow* init()
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
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD before calling any OpenGL functions.
	// glfwGetProcAddress gets the OS-specific OpenGL function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	// Set the callback function for automatically setting the viewport
	// when the window is resized.
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Render as wireframe.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Enable blending for transparency in textures.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

// Set the viewport to determine the size of the rendering window.
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Respond to user input.
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// Prepare the square's vertices to render.
void prepareSquare()
{
	// Create the vertex array object and then bind to it.
	// All subsequent VBO and EBO configurations will be stored in the VAO.
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Prepare the VBO.
	// Each vertex has a position (x, y, z), a colour (r, g, b), and texture coords (u, v).
	float vertices[] = {
		0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top right
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom left
		-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f // top left
	};

	glGenBuffers(1, &VBO);

	// Copy the vertices to a buffer for OpenGL.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the vertex attributes, stride is 6 (3 for position, 3 for colour).
	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Colour attribute, offset 3 from position.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
		(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texture coordinate attribute, offset 6 from position.
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Prepare the EBO.
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3 // second triangle
	};

	glGenBuffers(1, &EBO);

	// Copy the indices to a buffer for OpenGL.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void render()
{
	// Clear the colour buffer.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind to the approriate VAO.
	glBindVertexArray(VAO);

	// Draw the square with 6 vertices.
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// Unbind the VAO.
	glBindVertexArray(0);
}

// Deallocate all resources and terminate GLFW.
void cleanUp()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
}