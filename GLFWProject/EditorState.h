#pragma once
#ifndef EditorState_H
#define EditorState_H

#include "GameState.h"
#include "RoomData.h"

#include <memory>
#include <vector>

class Font;
class Shader;
class Texture;
class Room;

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
		MENU_RESIZE,
		MENU_TILES,
		MENU_LAYERS,
		MENU_LAYOUT
	};

	enum ResizeDir
	{
		TOP_LEFT,
		TOP_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_RIGHT
	};

	// Disallow instantiating.
	EditorState() {}

	// Save the current room write to external files.
	void save();

	// Select a tile on the room and perform an action depending on
	// the currently selected menu.
	void selectTile(const glm::ivec2 &windowSize);

	// Create the editor UI.
	void createUI(AssetLoader *assetLoader, SpriteRenderer *sRenderer);

	// Resize the current room.
	void resizeRoom(SpriteRenderer *sRenderer);

	// Change the currently selected layer and populate input fields.
	void selectLayer(int id);

	// Set the position for the current layer.
	void setRoomLayerPos(Room *room, glm::ivec2 pos);

	// Update the room with the editor's version of tiles texture and 
	// layout vector.
	void updateRoom();

	// Singleton instance.
	static EditorState m_state;

	// Hold pointers to assets.
	std::shared_ptr<Font> m_font;
	std::shared_ptr<Texture> m_tileset;

	bool m_isFirstFrame{ false };

	bool m_isEditorActive{ false };

	// Property input fields.
	int m_currentMenu{ MENU_PROPERTIES };
	char m_nameInput[128];
	char m_bgTextureInput[128];
	char m_musicInput[128];
	char m_shaderInput[128];

	// Resize edit fields.
	glm::ivec2 m_roomSize;
	glm::ivec2 m_newRoomSize;
	ResizeDir m_resizeDir;

	std::shared_ptr<Shader> m_roomShader{ nullptr };

	// The currently selected tile for the tile selector.
	const glm::ivec2 ERASER_TILE{ 255, 255 };
	glm::ivec2 m_tileToPlace{ ERASER_TILE };
	glm::ivec2 m_prevTileToPlace{ ERASER_TILE };

	// The currently selected type for the layout selector.
	glm::ivec2 m_typeToPlace{ ERASER_TILE };
	glm::ivec2 m_prevTypeToPlace{ ERASER_TILE };

	// Save the mouse position and if the mouse was clicked.
	glm::vec2 m_mousePos;
	bool m_isLeftClicked{ false };
	bool m_isRightClicked{ false };

	// The tile coordinate of the clicked tile.
	glm::ivec2 m_clickedTile{ -1 };
	glm::ivec2 m_prevClickedTile;

	// Hold the layout and layout texture for editing.
	std::vector<RoomData::TileType> m_roomLayout;
	std::shared_ptr<Texture> m_layoutTexture;

	// Hold the room's tiles texture for swapping with layout texture.
	std::vector<int> m_tiles;
	std::shared_ptr<Texture> m_tilesTexture;

	// Hold the room's layers and input fields for editing.
	std::vector<RoomData::Layer> m_roomLayers;
	int m_selectedLayerId{ -1 };
	char m_layerSpriteSheetInput[128];
	char m_layerTypeInput[128];
	int m_layerPosInput[2];
	float m_layerDepthInput;

	// Camera movement variables.
	glm::vec3 m_cameraPos;
	bool m_isMovingLeft;
	bool m_isMovingRight;
	bool m_isMovingUp;
	bool m_isMovingDown;
};

#endif