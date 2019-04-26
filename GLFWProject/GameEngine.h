#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

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
	void update();

	// Render all appropriate visuals for the game loop's current iteration.
	void render(Renderer *renderer);

	// The window to render to.
	GLFWwindow *m_window = nullptr;

	// The time between the current and last frame.
	float m_deltaTime = 0.0f;

	// The time of the last frame.
	float m_lastFrame = 0.0f;

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;
};

#endif