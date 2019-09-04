#pragma once
#ifndef PlayState_H
#define PlayState_H

#include "GameState.h"
#include "Camera.h"

#include <memory>
#include <string>

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
		TextRenderer *tRenderer, SoLoud::Soloud &soundEngine);

	// Render all appropriate visuals for the game loop's current iteration.
	virtual void render(const glm::ivec2 &windowSize, 
		SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer);

	// Getter functions.
	Camera *getCamera() const;
	Room *getCurrentRoom() const;

	// Prepare to change the room in the next update to the given room name.
	void changeRoom(const std::string &nextRoomName);

private:
	// Disallow instantiating.
	PlayState() {}

	// Change the current room to the next room.
	void changeRoom(AssetLoader *assetLoader, SpriteRenderer *sRenderer,
		SoLoud::Soloud &soundEngine);

	// Singleton instance.
	static PlayState m_state;

	// The camera to get the view matrix from.
	std::unique_ptr<Camera> m_camera;

	std::string m_roomName;
	std::shared_ptr<Room> m_currentRoom;
	std::shared_ptr<Font> m_font;

	// The name of the room to change to.
	std::string m_nextRoomName;
};

#endif