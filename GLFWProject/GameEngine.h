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

	// Update the camera to look at a position on the screen.
	void updateCameraLook(glm::vec2 screenPos);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// Getter functions.
	GLFWwindow *getWindow() const;
	Camera *getCamera() const;
	Room *getCurrentRoom() const;

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput(InputManager *inputManager);

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

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;

	// Flag for if the window size was changed.
	bool m_hasNewWindowSize{ true };

	// Flag for if debug mode is enabled.
	bool m_isDebugMode{ false };

	// Hold the frame rate.
	int m_fps;

	// TODO: remove this later for a more flexible implementation.
	std::shared_ptr<Room> m_currentRoom;
	std::shared_ptr<Font> m_font;
};

#endif