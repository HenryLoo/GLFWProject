#pragma once
#ifndef GameEngine_H
#define GameEngine_H

#include "Camera.h"
#include "GameComponent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

class Renderer;
class SpriteRenderer;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	// Start the game loop.
	// The function will only return when the game ends.
	void start(SpriteRenderer *renderer);

	// Update the camera to look at a position on the screen.
	void updateCameraLook(glm::vec2 screenPos);

	// Update the screen size for the renderer on the next iteration of the game loop.
	void updateRendererSize();

	// The maximum number of entities supported by the game.
	static const int MAX_ENTITIES{ 100000 };

private:
	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	void processInput();

	// Update all appropriate values for the game loop's current iteration.
	void update(SpriteRenderer *renderer);

	// Render all appropriate visuals for the game loop's current iteration.
	void render(SpriteRenderer *renderer);

	// Get the index of an unused entity.
	int findUnusedEntity();

	// TODO: test function for generating entities, remove this later.
	void createNewEntities();

	// The window to render to.
	GLFWwindow *m_window{ nullptr };

	// The time between the current and last frame.
	float m_deltaTime{ 0.0f };

	// The time of the last frame.
	float m_lastFrame{ 0.0f };

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;

	// Flag for if the window size was changed.
	bool m_hasNewWindowSize{ true };

	// Hold bitmasks that determines each entity's components.
	// If the value is 0, then the entity is dead.
	unsigned long m_entities[MAX_ENTITIES];

	// Hold components for each entity.
	GameComponent::Physics m_compPhysics[MAX_ENTITIES];
	GameComponent::Sprite m_compSprites[MAX_ENTITIES];

	// Hold the index of the last used entity.
	int m_lastUsedEntity{ 0 };
};

#endif