#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

class Room;
class AssetLoader;
class EntityManager;
class InputManager;
class SpriteRenderer;
class UIRenderer;
class TextRenderer;
class Font;
class GameState;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start(EntityManager *entityManager, AssetLoader *assetLoader, 
		InputManager *inputManager, SpriteRenderer *sRenderer, 
		UIRenderer *uRenderer, TextRenderer *tRenderer);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// Pop the last state from the stack and push on a new one.
	void changeState(GameState *state, AssetLoader *assetLoader);

	// Push a new state onto the stack.
	void pushState(GameState *state, AssetLoader *assetLoader);

	// Pop the top-most state from the stack.
	void popState();

	// Getter functions.
	GLFWwindow *getWindow() const;

	// Quit the game.
	void quit() const;

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput(InputManager *inputManager, EntityManager *entityManager, 
		AssetLoader *assetLoader);

	// Update all appropriate values for the game loop's current iteration.
	void update(EntityManager *entityManager, AssetLoader *assetLoader,
		SpriteRenderer *sRenderer, UIRenderer *uRenderer, 
		TextRenderer *tRenderer);

	// Render all appropriate visuals for the game loop's current iteration.
	void render(SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer);

	// The window to render to.
	GLFWwindow *m_window{ nullptr };
	glm::ivec2 m_windowSize;

	// The time between the current and last frame.
	float m_deltaTime{ 0.0f };

	// The time of the last frame.
	float m_lastFrame{ 0.0f };

	// The state stack.
	std::vector<GameState *> m_states{};

	// Flag for if the window size was changed.
	bool m_hasNewWindowSize{ true };

	// Flag for if debug mode is enabled.
	bool m_isDebugMode{ false };
	std::shared_ptr<Font> m_debugFont;

	// Hold the frame rate.
	int m_fps;
};

#endif