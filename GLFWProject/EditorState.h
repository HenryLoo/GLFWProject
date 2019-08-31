#pragma once
#ifndef EditorState_H
#define EditorState_H

#include "GameState.h"

#include <memory>

class Font;
class Shader;
class Texture;

class EditorState : public GameState
{
public:
	// Return the singleton instance.
	static EditorState *instance();

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
	enum MenuId
	{
		MENU_PROPERTIES,
		MENU_TILES,
		MENU_LAYERS,
		MENU_LAYOUT
	};

	// Disallow instantiating.
	EditorState() {}

	// Save the current room write to external files.
	void save();

	// Place a tile on the room.
	void placeTile(const glm::ivec2 &windowSize);

	// Create the editor UI.
	void createUI();

	// Singleton instance.
	static EditorState m_state;

	// Hold pointers to assets.
	std::shared_ptr<Font> m_font;
	std::shared_ptr<Texture> m_tileset;

	bool m_isEditorActive{ false };

	// Property input fields.
	int m_currentMenu{ MENU_PROPERTIES };
	char m_nameInput[128];
	char m_tilesInput[128];
	char m_bgTextureInput[128];
	char m_musicInput[128];
	char m_shaderInput[128];

	// The currently selected tile for the tile selector.
	const glm::ivec2 ERASER_TILE{ 255, 255 };
	glm::ivec2 m_tileToPlace{ ERASER_TILE };
	glm::ivec2 m_prevTileToPlace{ ERASER_TILE };

	// Save the mouse position and if the mouse was clicked.
	glm::vec2 m_mousePos;
	bool m_isLeftClicked{ false };
	bool m_isRightClicked{ false };

	// The tile coordinate of the clicked tile.
	glm::ivec2 m_clickedTile{ -1 };
	glm::ivec2 m_prevClickedTile;
};

#endif