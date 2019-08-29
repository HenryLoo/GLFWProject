#pragma once
#ifndef GameState_H
#define GameState_H

#include <glm/glm.hpp>

class InputManager;
class EntityManager;
class AssetLoader;
class SpriteRenderer;
class UIRenderer;
class TextRenderer;
class GameEngine;

namespace SoLoud
{
	class Soloud;
}


class GameState
{
public:
	// Initialize and clean up.
	virtual void init(AssetLoader *assetLoader) = 0;
	virtual void cleanUp() = 0;

	// Pause and resume the state.
	virtual void pause() = 0;
	virtual void resume() = 0;

	// Constructor is private to prevent instantiating singleton.
	// Handle all user inputs for the game loop's current iteration.
	virtual void processInput(GameEngine *game, 
		InputManager *inputManager, EntityManager *entityManager, 
		AssetLoader *assetLoader) = 0;

	// Update all appropriate values for the game loop's current iteration.
	virtual void update(float deltaTime, const glm::ivec2 &windowSize, 
		EntityManager *entityManager, AssetLoader *assetLoader, 
		SpriteRenderer *sRenderer, UIRenderer *uRenderer, 
		TextRenderer *tRenderer, SoLoud::Soloud &soundEngine) = 0;

	// Render all appropriate visuals for the game loop's current iteration.
	virtual void render(const glm::ivec2 &windowSize, 
		SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer) = 0;
};

#endif