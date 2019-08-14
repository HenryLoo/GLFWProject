#include "PlayState.h"

#include "EntityManager.h"
#include "AssetLoader.h"
#include "SpriteRenderer.h"
#include "UIRenderer.h"
#include "TextRenderer.h"
#include "Camera.h"
#include "Room.h"
#include "Font.h"
#include "InputManager.h"
#include "GameEngine.h"
#include "MenuState.h"

#include <iostream>

PlayState PlayState::m_state;

PlayState *PlayState::instance()
{
	return &m_state;
}

void PlayState::init(AssetLoader *assetLoader)
{
	// TODO: remove this later for more flexible approach.
	if (m_currentRoom == nullptr)
		m_currentRoom = assetLoader->load<Room>("test");
	if (m_font == nullptr)
		m_font = assetLoader->load<Font>("default", 16);

	// Initialize the camera.
	m_camera = std::make_unique<Camera>();
}

void PlayState::cleanUp()
{

}

void PlayState::pause()
{

}

void PlayState::resume()
{

}

void PlayState::processInput(GameEngine *game, InputManager *inputManager,
	AssetLoader *assetLoader)
{
	// Pause game.
	if (inputManager->isKeyPressed(InputManager::INPUT_CANCEL, true))
	{
		game->pushState(MenuState::instance(), assetLoader);
	}
}

void PlayState::update(float deltaTime, const glm::ivec2 &windowSize, 
	EntityManager *entityManager, AssetLoader *assetLoader, 
	SpriteRenderer *sRenderer, UIRenderer *uRenderer, 
	TextRenderer *tRenderer)
{	
	// Clear the renderer data, since updating may repopulate these.
	sRenderer->resetData();
	uRenderer->resetData();
	tRenderer->resetData();

	glm::vec3 playerPos{ entityManager->getPlayerPos() };
	if (m_currentRoom != nullptr)
	{
		m_camera->update(deltaTime, playerPos, windowSize,
			m_currentRoom->getSize());
	}

	// Update all entities.
	entityManager->update(deltaTime, assetLoader, uRenderer, tRenderer);

	Renderer::updateViewMatrix(m_camera->getViewMatrix());
}

void PlayState::render(const glm::ivec2 &windowSize, 
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer)
{
	// Render queued sprites.
	sRenderer->render(m_camera.get(), m_currentRoom.get());

	// Render UI elements.
	uRenderer->renderHud();

	// Render queued text.
	tRenderer->render();
}

Camera *PlayState::getCamera() const
{
	return m_camera.get();
}

Room *PlayState::getCurrentRoom() const
{
	return m_currentRoom.get();
}