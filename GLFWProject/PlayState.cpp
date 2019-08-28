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
#include "Prefab.h"

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
	{
		std::shared_ptr<Prefab> roomPrefab{ assetLoader->load<Prefab>("room_test") };
		m_currentRoom = std::make_unique<Room>(roomPrefab.get(), assetLoader);
	}
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
	EntityManager *entityManager, AssetLoader *assetLoader)
{
	// Pause game.
	if (inputManager->isKeyPressed(InputManager::INPUT_CANCEL, true))
	{
		game->pushState(MenuState::instance(), assetLoader);
	}

	// Debug 2: create enemy.
	if (inputManager->isKeyPressed(InputManager::INPUT_DEBUG2, true))
	{
		entityManager->createEntity("clamper", glm::vec2(128.f, 300.f));
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

	// Update all entities.
	entityManager->update(deltaTime, assetLoader, uRenderer, tRenderer);

	// Update the camera after processing entity systems, since the
	// camera follows the player's position.
	if (m_currentRoom != nullptr)
	{
		// Update room layers.
		m_currentRoom->updateLayers(sRenderer);

		glm::vec3 playerPos{ entityManager->getPlayerPos() };
		m_camera->update(deltaTime, playerPos, windowSize,
			m_currentRoom->getSize());
	}
}

void PlayState::render(const glm::ivec2 &windowSize, 
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer)
{
	// Update the view matrix for all renderers.
	Renderer::updateViewMatrix(m_camera->getViewMatrix());

	// Render queued sprites.
	sRenderer->render(m_camera.get(), m_currentRoom.get(),
		m_currentRoom->getShader());

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