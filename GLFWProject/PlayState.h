#pragma once
#ifndef PlayState_H
#define PlayState_H

#include "GameState.h"
#include "Camera.h"

#include <memory>

class Room;
class Font;

class PlayState : public GameState
{
public:
	// Return the singleton instance.
	static PlayState *instance();

	// Initialize and clean up.
	virtual void init(AssetLoader *assetLoader);
	virtual void cleanUp();

	// Pause and resume the state.
	virtual void pause();
	virtual void resume();

	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	virtual void processInput(GameEngine *game, InputManager *inputManager, 
		EntityManager *entityManager, AssetLoader *assetLoader);

	// Update all appropriate values for the game loop's current iteration.
	virtual void update(float deltaTime, const glm::ivec2 &windowSize,
		EntityManager *entityManager,  AssetLoader *assetLoader,
		SpriteRenderer *sRenderer,  UIRenderer *uRenderer, 
		TextRenderer *tRenderer);

	// Render all appropriate visuals for the game loop's current iteration.
	virtual void render(const glm::ivec2 &windowSize, 
		SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer);

	// Getter functions.
	Camera *getCamera() const;
	Room *getCurrentRoom() const;

private:
	// Disallow instantiating.
	PlayState() {}

	// Singleton instance.
	static PlayState m_state;

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;

	std::shared_ptr<Room> m_currentRoom;
	std::shared_ptr<Font> m_font;
};

#endif