#include "MenuState.h"

#include "AssetLoader.h"
#include "TextRenderer.h"
#include "Font.h"
#include "InputManager.h"
#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "PlayState.h"
#include "Shader.h"

#include <iostream>

MenuState MenuState::m_state;

MenuState *MenuState::instance()
{
	return &m_state;
}

void MenuState::init(AssetLoader *assetLoader)
{
	if (m_font == nullptr)
		m_font = assetLoader->load<Font>("default", 16);
	if (m_pausedShader == nullptr)
		m_pausedShader = assetLoader->load<Shader>("paused");
}

void MenuState::cleanUp()
{

}

void MenuState::pause()
{

}

void MenuState::resume()
{

}

void MenuState::processInput(GameEngine *game, InputManager *inputManager, 
	EntityManager *entityManager, AssetLoader *assetLoader)
{
	// Unpause game.
	if (inputManager->isKeyPressed(InputManager::INPUT_CANCEL, true))
	{
		game->popState();
	}

	// TODO: Quit game. Change this to something else.
	if (inputManager->isKeyPressed(InputManager::INPUT_JUMP, true))
	{
		game->quit();
	}
}

void MenuState::update(float deltaTime, const glm::ivec2 &windowSize,
	EntityManager *entityManager, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer, SoLoud::Soloud &soundEngine)
{
	// Clear the renderer data, since updating may repopulate these.
	tRenderer->resetData();

	tRenderer->addText("GAME PAUSED", m_font.get(),
		{ { 0.f, 0.f }, windowSize }, TextAlign::CENTER, true);
}

void MenuState::render(const glm::ivec2 &windowSize,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer)
{
	// Render sprites from PlayState, freeze-framed.
	PlayState *playState{ PlayState::instance() };
	Camera *camera{ playState->getCamera() };
	Room *room{ playState->getCurrentRoom() };
	if (camera != nullptr && room != nullptr)
	{
		sRenderer->render(camera, room, m_pausedShader.get());
	}

	// Render queued text.
	tRenderer->render();
}