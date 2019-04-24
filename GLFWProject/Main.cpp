#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// Forward declarations.
GLFWwindow* init();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void prepareSquare();
void loadShaders();
void render();

unsigned int VBO, vertexShader, fragmentShader, shaderProgram, VAO, EBO;

const char* vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n\0";


int main()
{
	GLFWwindow* window = init();

	// Exit if the window could not be created.
	if (window == nullptr)
		return -1;


	// Load the shaders.
	loadShaders();

	// Prepare the triangle's vertices.
	prepareSquare();

	// Render loop.
	while (!glfwWindowShouldClose(window))
	{
		// Handle user inputs.
		processInput(window);

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
	float vertices[] = {
		0.5f, 0.5f, 0.0f, // top right
		0.5f, -0.5f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, // bottom left
		-0.5f, 0.5f, 0.0f // top left
	};

	glGenBuffers(1, &VBO);

	// Copy the vertices to a buffer for OpenGL.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the vertex attributes.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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

// Load the appropriate vertex and fragment shaders.
void loadShaders()
{
	// Create the vertex shader and store its id.
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Attach the shader source code to the shader object, and then compile it.
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check if the shader compiled properly.
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Vertex shader failed to compile\n" << infoLog <<
			std::endl;
	}

	// Create the fragment shader and store its id.
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check if the shader compiled properly.
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Fragment shader failed to compile\n" << infoLog <<
			std::endl;
	}

	// Create the shader program and store its id.
	shaderProgram = glCreateProgram();

	// Attach the compiled vertex and fragment shaders to the program.
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check if the program compiled properly.
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "Shader program failed to compile\n" << infoLog <<
			std::endl;
	}

	// Clean up by deleting the shader objects.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void render()
{
	// Clear the colour buffer.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set the shader program.
	glUseProgram(shaderProgram);

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