#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start(Renderer *renderer);

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput();

	// Update all appropriate values for the game loop's current iteration.
	void update(float deltaTime);

	// Render all appropriate visuals for the game loop's current iteration.
	void render(Renderer *renderer);

	// The window to render to.
	GLFWwindow *m_window = nullptr;
};

#endif