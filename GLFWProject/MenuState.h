#pragma once
#ifndef MenuState_H
#define MenuState_H

#include "GameState.h"

#include <memory>

class Font;
class Shader;

class MenuState : public GameState
{
public:
	// Return the singleton instance.
	static MenuState *instance();

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
		EntityManager *entityManager, AssetLoader *assetLoader,
		SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer, SoLoud::Soloud &soundEngine);

	// Render all appropriate visuals for the game loop's current iteration.
	virtual void render(const glm::ivec2 &windowSize,
		SpriteRenderer *sRenderer, UIRenderer *uRenderer,
		TextRenderer *tRenderer);

private:
	// Disallow instantiating.
	MenuState() {}

	// Singleton instance.
	static MenuState m_state;

	// Hold pointers to assets.
	std::shared_ptr<Font> m_font;
	std::shared_ptr<Shader> m_pausedShader;
};

#endif