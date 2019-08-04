#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"
#include "SpriteRenderer.h"
#include "UIRenderer.h"
#include "InputManager.h"
#include "EntityManager.h"
#include "AssetLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

class Renderer;
class SpriteRenderer;
class UIRenderer;
class InputManager;
class Room;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start();

	// Update the camera to look at a position on the screen.
	void updateCameraLook(glm::vec2 screenPos);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// Getter functions.
	SpriteRenderer *getSpriteRenderer() const;
	UIRenderer *getUIRenderer() const;
	InputManager *getInputManager() const;
	Camera *getCamera() const;
	Room *getCurrentRoom() const;

	// Load an asset from the asset loader.
	template <typename T>
	std::shared_ptr<T> loadAsset(const std::string name,
		const std::vector<std::string> &filePaths);

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput();

	// Update all appropriate values for the game loop's current iteration.
	void update();

	// Render all appropriate visuals for the game loop's current iteration.
	void render();

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

	std::unique_ptr<SpriteRenderer> m_sRenderer;
	std::unique_ptr<UIRenderer> m_uRenderer;
	std::unique_ptr<InputManager> m_input;
	std::unique_ptr<EntityManager> m_entityManager;
	std::unique_ptr<AssetLoader> m_assetLoader;

	// TODO: remove this later for a more flexible implementation.
	std::shared_ptr<Room> m_currentRoom;
};

template <typename T>
std::shared_ptr<T> GameEngine::loadAsset(const std::string name,
	const std::vector<std::string> &filePaths)
{
	return m_assetLoader->load<T>(name, filePaths);
}

#endif