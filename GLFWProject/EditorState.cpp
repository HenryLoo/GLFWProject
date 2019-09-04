#include "EditorState.h"

#include "AssetLoader.h"
#include "TextRenderer.h"
#include "Font.h"
#include "InputManager.h"
#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "PlayState.h"
#include "Shader.h"
#include "JSONUtilities.h"
#include "Texture.h"
#include "RoomData.h"

#include <json/single_include/nlohmann/json.hpp>

#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <fstream>
#include <iomanip>

namespace
{
	const std::string OUTPUT_PATH{ "editor/" };

	const glm::ivec2 TILE_SELECTOR_SIZE{ 6, 9 };
}

EditorState EditorState::m_state;

EditorState *EditorState::instance()
{
	return &m_state;
}

void EditorState::init(AssetLoader *assetLoader)
{
	if (m_font == nullptr)
		m_font = assetLoader->load<Font>("default", 16);
	if (m_tileset == nullptr)
		m_tileset = assetLoader->load<Texture>("tileset");

	PlayState *playState{ PlayState::instance() };
	Room *room{ playState->getCurrentRoom() };
	if (room != nullptr)
	{
		const RoomData::Data &roomData{ room->getRoomData() };
		strcpy_s(m_nameInput, sizeof(m_nameInput), roomData.name.c_str());
		strcpy_s(m_bgTextureInput, sizeof(m_bgTextureInput), roomData.bgTextureName.c_str());
		strcpy_s(m_musicInput, sizeof(m_musicInput), roomData.musicName.c_str());
		strcpy_s(m_shaderInput, sizeof(m_shaderInput), roomData.shaderName.c_str());

		m_roomShader = assetLoader->load<Shader>(roomData.shaderName);

		m_tiles = roomData.tiles;
		m_tilesTexture = room->getTileSprites();

		m_roomSize = m_newRoomSize = room->getSize();
		m_roomLayout = roomData.layout;

		// Show layout texture if the current menu is the layout selector.
		if (m_currentMenu == MENU_LAYOUT)
			room->setTilesTexture(m_layoutTexture);

		// Get room layers.
		m_roomLayers = roomData.layers;
		if (m_selectedLayerId == -1 && roomData.layers.size() > 0)
		{
			// Select the first layer if no layers selected.
			selectLayer(0);
		}

		Camera *camera{ playState->getCamera() };
		if (camera != nullptr)
		{
			m_cameraPos = camera->getPosition();
		}
	}

	m_isFirstFrame = true;
}

void EditorState::cleanUp()
{
	updateRoom();
}

void EditorState::pause()
{

}

void EditorState::resume()
{

}

void EditorState::processInput(GameEngine *game, InputManager *inputManager,
	EntityManager *entityManager, AssetLoader *assetLoader)
{
	// Unpause game.
	if (inputManager->isKeyPressed(InputManager::INPUT_CANCEL, true))
	{
		game->popState();
	}

	// Get the mouse position.
	m_mousePos = inputManager->getMousePos();
	m_isLeftClicked = inputManager->isMousePressed(GLFW_MOUSE_BUTTON_LEFT);
	m_isRightClicked = inputManager->isMousePressed(GLFW_MOUSE_BUTTON_RIGHT);

	m_isMovingLeft = m_isMovingRight = m_isMovingUp = m_isMovingDown = false;
	if (inputManager->isKeyPressing(InputManager::INPUT_LEFT))
	{
		m_isMovingLeft = true;
	}
	if (inputManager->isKeyPressing(InputManager::INPUT_RIGHT))
	{
		m_isMovingRight = true;
	}
	if (inputManager->isKeyPressing(InputManager::INPUT_UP))
	{
		m_isMovingUp = true;
	}
	if (inputManager->isKeyPressing(InputManager::INPUT_DOWN))
	{
		m_isMovingDown = true;
	}
}

void EditorState::update(float deltaTime, const glm::ivec2 &windowSize,
	EntityManager *entityManager, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer, SoLoud::Soloud &soundEngine)
{
	// Set up room layout texture.
	if (m_isFirstFrame)
	{
		m_layoutTexture = Room::createTilesTexture(sRenderer, m_roomSize, 
			m_roomLayout);
		m_isFirstFrame = false;
	}

	sRenderer->resetData();

	// Update room layers.
	PlayState *playState{ PlayState::instance() };
	Room *room{ playState->getCurrentRoom() };
	if (room != nullptr)
	{
		// Update room layers.
		room->updateLayers(sRenderer);

		const float cameraSpeed{ 256.f };
		if (m_isMovingRight)
			m_cameraPos.x += (cameraSpeed * deltaTime);
		else if (m_isMovingLeft)
			m_cameraPos.x -= (cameraSpeed * deltaTime);

		if (m_isMovingUp)
			m_cameraPos.y += (cameraSpeed * deltaTime);
		else if (m_isMovingDown)
			m_cameraPos.y -= (cameraSpeed * deltaTime);

		Camera *camera{ playState->getCamera() };
		if (camera != nullptr)
		{
			glm::ivec2 roomSize{ room->getSize() };
			glm::ivec2 windowHalfSize{ windowSize / 2 };
			glm::ivec2 roomSizePixel{ roomSize * Room::TILE_SIZE };
			float zoom{ camera->getZoom() };
			m_cameraPos.x = glm::clamp(m_cameraPos.x,
				windowHalfSize.x / zoom,
				roomSizePixel.x - windowHalfSize.x / zoom);
			m_cameraPos.y = glm::clamp(m_cameraPos.y,
				windowHalfSize.y / zoom,
				roomSizePixel.y - windowHalfSize.y / zoom);

			camera->update(deltaTime, m_cameraPos, windowSize,
				room->getSize(), true);
		}

		Renderer::updateViewMatrix(camera->getViewMatrix());
	}

	createUI(assetLoader, sRenderer);
	selectTile(windowSize);
}

void EditorState::render(const glm::ivec2 &windowSize,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer)
{
	// Render sprites from PlayState, freeze-framed.
	PlayState *playState{ PlayState::instance() };
	Camera *camera{ playState->getCamera() };
	Room *room{ playState->getCurrentRoom() };
	if (camera != nullptr && room != nullptr)
	{
		// Only show the room shader if the menu is at properties.
		Shader *shader{ nullptr };
		if (m_currentMenu == MENU_PROPERTIES || m_currentMenu == MENU_RESIZE)
			shader = m_roomShader.get();

		sRenderer->render(camera, room, shader);
	}

	// Render ImGui.
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorState::save()
{
	PlayState *playState{ PlayState::instance() };
	Room *room{ nullptr };
	if (playState != nullptr)
		room = playState->getCurrentRoom();

	nlohmann::json json;
	if (room != nullptr)
	{
		// Write the room's JSON file.
		JSONUtilities::roomToJson(room->getRoomData(), json);

		std::ofstream o(OUTPUT_PATH + m_nameInput + ".json");
		o << json << std::endl;
		o.close();
	}
}

void EditorState::selectTile(const glm::ivec2 &windowSize)
{
	if (m_currentMenu != MENU_TILES && 
		m_currentMenu != MENU_LAYOUT &&
		m_currentMenu != MENU_LAYERS)
		return;

	m_prevClickedTile = m_clickedTile;

	// Don't allow clicking through the editor UI.
	if (m_isLeftClicked &&
		!ImGui::IsAnyWindowFocused())
	{
		Camera *camera{ PlayState::instance()->getCamera() };
		Room *room{ PlayState::instance()->getCurrentRoom() };
		if (camera != nullptr && room != nullptr)
		{
			glm::vec4 clipPos{ m_mousePos.x / windowSize.x * 2.f - 1.f,
				-1.f * (m_mousePos.y / windowSize.y * 2.f - 1.f), 0.f, 1.f };
			clipPos = glm::clamp(clipPos, -1.f, 1.f);

			glm::mat4 projectionMatrix{ Renderer::getOrthographicMatrix(camera->getZoom()) };
			glm::vec4 viewPos{ glm::inverse(projectionMatrix) * clipPos };

			glm::mat4 viewMatrix{ camera->getViewMatrix() };
			glm::vec4 worldPos{ glm::inverse(viewMatrix) * viewPos };

			m_clickedTile = room->getTileCoord(glm::vec2(worldPos.x, worldPos.y));

			// Placing tile sprite with tiles selector.
			if (m_currentMenu == MENU_TILES && (m_prevClickedTile != m_clickedTile || 
				m_prevTileToPlace != m_tileToPlace))
			{
				placeTile(m_prevTileToPlace, m_tileToPlace, 
					m_tilesTexture.get(), m_tiles);
			}
			// Placing tile type with layout selector.
			else if (m_currentMenu == MENU_LAYOUT && (m_prevClickedTile != m_clickedTile ||
				m_prevTypeToPlace != m_typeToPlace))
			{
				placeTile(m_prevTypeToPlace, m_typeToPlace,
					m_layoutTexture.get(), m_roomLayout);
			}
			// Moving the selected layer.
			else if (m_currentMenu == MENU_LAYERS && m_prevClickedTile != m_clickedTile)
			{
				m_layerPosInput[0] = m_clickedTile.x;
				m_layerPosInput[1] = m_clickedTile.y;
				setRoomLayerPos(room, m_clickedTile);
			}
		}
	}
}

void EditorState::placeTile(glm::ivec2 &prevTile, glm::ivec2 &tileToPlace,
	Texture *texToEdit, std::vector<int> &tilesToEdit)
{
	prevTile = tileToPlace;

	// Change the pixel on the texture.
	if (texToEdit != nullptr)
	{
		char pixel[4]{ (char)tileToPlace.x, (char)tileToPlace.y,
			(char)0, (char)255 };
		texToEdit->bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, m_clickedTile.x,
			m_clickedTile.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	}

	// Change the tile configuration.
	int tile{ 0 };
	if (tileToPlace != ERASER_TILE)
		tile = tileToPlace.x + 1;
	Room *room{ PlayState::instance()->getCurrentRoom() };
	int tileIndex{ room->getTileIndex(m_clickedTile) };
	tilesToEdit[tileIndex] = tile;
}

void EditorState::createUI(AssetLoader *assetLoader, SpriteRenderer *sRenderer)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Room Editor", &m_isEditorActive,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_AlwaysAutoResize);

	Room *room{ PlayState::instance()->getCurrentRoom() };
	bool hasRoom{ room != nullptr };
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save")) 
			{
				updateRoom(); 
				save(); 
			}
			if (ImGui::MenuItem("Reload Room")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Properties"))
			{ 
				m_currentMenu = MENU_PROPERTIES; 
				if (hasRoom && m_tilesTexture != nullptr)
					room->setTilesTexture(m_tilesTexture);
			}
			if (ImGui::MenuItem("Resize"))
			{
				m_currentMenu = MENU_RESIZE;
				if (hasRoom && m_tilesTexture != nullptr)
					room->setTilesTexture(m_tilesTexture);
			}
			if (ImGui::MenuItem("Tiles")) 
			{ 
				m_currentMenu = MENU_TILES; 
				if (hasRoom && m_tilesTexture != nullptr)
					room->setTilesTexture(m_tilesTexture);
			}
			if (ImGui::MenuItem("Layers"))
			{ 
				m_currentMenu = MENU_LAYERS; 

				if (hasRoom && m_tilesTexture != nullptr)
					room->setTilesTexture(m_tilesTexture);
			}
			if (ImGui::MenuItem("Layout"))
			{
				m_currentMenu = MENU_LAYOUT;
				if (hasRoom && m_layoutTexture != nullptr)
					room->setTilesTexture(m_layoutTexture);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	switch (m_currentMenu)
	{
		case MENU_PROPERTIES:
		{
			ImGui::InputText("Name", m_nameInput, IM_ARRAYSIZE(m_nameInput));
			ImGui::InputText("Background", m_bgTextureInput, IM_ARRAYSIZE(m_bgTextureInput));
			ImGui::InputText("Music", m_musicInput, IM_ARRAYSIZE(m_musicInput));
			ImGui::InputText("Shader", m_shaderInput, IM_ARRAYSIZE(m_shaderInput));

			break;
		}

		case MENU_RESIZE:
		{
			ImGui::InputScalar("Width", ImGuiDataType_S32,
				&m_newRoomSize.x);
			ImGui::InputScalar("Height", ImGuiDataType_S32,
				&m_newRoomSize.y);
			ImGui::NewLine();

			ImGui::Text("Keep tiles at:");
			if (ImGui::Button("Top-Left"))
			{
				m_resizeDir = TOP_LEFT;
			}
			ImGui::SameLine();
			if (ImGui::Button("Top-Right"))
			{
				m_resizeDir = TOP_RIGHT;
			}

			if (ImGui::Button("Bottom-Left"))
			{
				m_resizeDir = BOTTOM_LEFT;
			}
			ImGui::SameLine();
			if (ImGui::Button("Bottom-Right"))
			{
				m_resizeDir = BOTTOM_RIGHT;
			}

			ImGui::NewLine();
			if (ImGui::Button("Resize"))
			{
				resizeRoom(sRenderer);
			}
			break;
		}

		case MENU_TILES:
		case MENU_LAYOUT:
		{
			bool isLayout{ m_currentMenu == MENU_LAYOUT };
			if (ImGui::Button("Eraser"))
			{
				if (isLayout)
				{
					m_prevTypeToPlace = m_typeToPlace;
					m_typeToPlace = ERASER_TILE;
				}
				else
				{
					m_prevTileToPlace = m_tileToPlace;
					m_tileToPlace = ERASER_TILE;
				}
			}

			glm::vec2 tilesetSize{ m_tileset->getSize() };
			ImTextureID textureId{ (void *)(intptr_t)m_tileset->getTextureId() };
			glm::ivec2 numTiles{ tilesetSize.x / Room::TILE_SIZE,
				tilesetSize.y / Room::TILE_SIZE };
			glm::vec2 tileSizeUV{ Room::TILE_SIZE / tilesetSize.x,
				Room::TILE_SIZE / tilesetSize.y };

			int currentCol{ 0 };
			int tileIndexOffset{ RoomData::NUM_TILE_TYPES - 1 };
			int totalNumTiles{ TILE_SELECTOR_SIZE.x * TILE_SELECTOR_SIZE.y };
			if (isLayout)
			{
				totalNumTiles = tileIndexOffset;
				tileIndexOffset = 0;
			}
			for (int i = tileIndexOffset; i < totalNumTiles; ++i)
			{
				// Calculate uv's for this tile.
				glm::ivec2 tilesetIndex{ i % numTiles.x, floor(i / numTiles.x) };
				glm::vec2 u{ tilesetIndex.x * tileSizeUV.x, (tilesetIndex.x + 1) * tileSizeUV.x };
				glm::vec2 v{ tilesetIndex.y * tileSizeUV.y, (tilesetIndex.y + 1) * tileSizeUV.y };

				// If already selected, change tint colour.
				ImVec4 tintColour{ 1.f, 1.f, 1.f, 1.f };
				if ((!isLayout && tilesetIndex == m_tileToPlace) ||
					(isLayout && tilesetIndex == m_typeToPlace))
					tintColour = ImVec4(1.f, 0.f, 0.f, 1.f);

				// Add the tile button.
				ImGui::PushID(i);
				if (ImGui::ImageButton(textureId,
					ImVec2(Room::TILE_SIZE * 2, Room::TILE_SIZE * 2),
					ImVec2(u.x, 1 - v.x), ImVec2(u.y, 1 - v.y), -1,
					ImVec4(0.f, 0.f, 0.f, 0.f), tintColour))
				{
					if (isLayout)
					{
						m_prevTypeToPlace = m_typeToPlace;
						m_typeToPlace = tilesetIndex;
					}
					else
					{
						m_prevTileToPlace = m_tileToPlace;
						m_tileToPlace = tilesetIndex;
					}
				}
				ImGui::PopID();

				// Determine if there should be a new line.
				if (currentCol < TILE_SELECTOR_SIZE.x - 1)
				{
					ImGui::SameLine();
					currentCol++;
				}
				else
				{
					currentCol = 0;
				}
			}

			break;
		}

		case MENU_LAYERS:
		{
			if (ImGui::ArrowButton("Left", ImGuiDir_Left))
			{
				int id{ m_selectedLayerId - 1 };
				if (id == -1)
					id = static_cast<int>(m_roomLayers.size() - 1);
				selectLayer(id);
			}
			ImGui::SameLine();
			if (ImGui::ArrowButton("Right", ImGuiDir_Right))
			{
				int id{ m_selectedLayerId + 1 };
				if (id == m_roomLayers.size())
					id = 0;
				selectLayer(id);
			}
			ImGui::SameLine();

			std::string numLayers{ "Selected Layer: " + 
				std::to_string(m_selectedLayerId + 1) + " / " + 
				std::to_string(m_roomLayers.size()) };
			ImGui::Text(numLayers.c_str());

			if (ImGui::Button("New Layer"))
			{
				room->addLayer();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				m_roomLayers.erase(m_roomLayers.begin() + m_selectedLayerId);
				room->deleteLayer(m_selectedLayerId);
			}
			ImGui::NewLine();

			if (ImGui::InputText("Spritesheet", m_layerSpriteSheetInput,
				IM_ARRAYSIZE(m_layerSpriteSheetInput), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				m_roomLayers[m_selectedLayerId].spriteSheetName = m_layerSpriteSheetInput;
				room->setLayerSpriteSheet(m_selectedLayerId, assetLoader, 
					m_layerSpriteSheetInput);
			}
			if (ImGui::InputText("Type", m_layerTypeInput,
				IM_ARRAYSIZE(m_layerTypeInput), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				m_roomLayers[m_selectedLayerId].type = m_layerTypeInput;
				room->setLayerType(m_selectedLayerId, m_layerTypeInput);
			}
			if (ImGui::InputScalarN("Position (x, y)", ImGuiDataType_S32,
				m_layerPosInput, 2))
			{
				setRoomLayerPos(room, glm::ivec2(m_layerPosInput[0],
					m_layerPosInput[1]));
			}
			if (ImGui::InputScalar("Depth", ImGuiDataType_Float,
				&m_layerDepthInput))
			{
				m_roomLayers[m_selectedLayerId].pos.z = m_layerDepthInput;
				room->setLayerDepth(m_selectedLayerId, m_layerDepthInput);
			}
			break;
		}
	}

	ImGui::End();

	ImGui::EndFrame();
}

// TODO: not working properly; need to fix this.
void EditorState::resizeRoom(SpriteRenderer *sRenderer)
{
	if (m_roomSize == m_newRoomSize)
		return;

	glm::ivec2 diff{ m_newRoomSize - m_roomSize };
	std::vector<int> layout;

	std::function<bool(int)> isNewTileX;
	std::function<bool(int)> isNewTileY;
	int firstX{ 0 }, firstY{ 0 };
	switch (m_resizeDir)
	{
		case TOP_LEFT:
		{
			isNewTileX = [this](int x) -> bool 
			{ 
				return x >= m_roomSize.x;
			};
			isNewTileY = [this](int y) -> bool
			{
				return y >= m_roomSize.y;
			};
			break;
		}
		case TOP_RIGHT:
		{
			if (diff.x < 0)
				firstX = -diff.x;

			isNewTileX = [this, diff](int x) -> bool
			{
				return x < diff.x;
			};
			isNewTileY = [this, diff](int y) -> bool
			{
				return y >= m_roomSize.y;
			};
			break;
		}
		case BOTTOM_LEFT:
		{
			if (diff.y < 0)
				firstY = -diff.y;

			isNewTileX = [this, diff](int x) -> bool
			{
				return x >= m_roomSize.x;
			};
			isNewTileY = [this, diff](int y) -> bool
			{
				return y < diff.y;
			};
			break;
		}
		case BOTTOM_RIGHT:
		{
			if (diff.x < 0)
				firstX = -diff.x;
			if (diff.y < 0)
				firstY = -diff.y;

			isNewTileX = [diff](int x) -> bool
			{
				return x < diff.x;
			};
			isNewTileY = [diff](int y) -> bool
			{
				return y < diff.y;
			};
			break;
		}
	}

	int x{ 0 }, y{ 0 };
	for (int i = firstY; i < m_newRoomSize.y + firstY; ++i)
	{
		for (int j = firstX; j < m_newRoomSize.x + firstX; ++j)
		{
			int tile{ 0 };
			if (!isNewTileX(j) && !isNewTileY(i))
			{
				tile = m_roomLayout[m_roomSize.x * y + x];
				++x;
			}

			layout.push_back(tile);
			std::cout << tile;
		}
		x = 0;

		if (!isNewTileY(i - 1))
		{
			++y;
		}
		std::cout << std::endl;
	}

	m_roomSize = m_newRoomSize;
	m_roomLayout = layout;

	// Update the layout texture.
	m_layoutTexture = Room::createTilesTexture(sRenderer, m_roomSize, 
		m_roomLayout);

	// Update the room.
	updateRoom();
}

void EditorState::selectLayer(int id)
{
	m_selectedLayerId = glm::clamp(id, 0, 
		static_cast<int>(m_roomLayers.size()));

	RoomData::Layer &thisLayer{ m_roomLayers[m_selectedLayerId] };
	strcpy_s(m_layerSpriteSheetInput, sizeof(m_layerSpriteSheetInput), 
		thisLayer.spriteSheetName.c_str());
	strcpy_s(m_layerTypeInput, sizeof(m_layerTypeInput),
		thisLayer.type.c_str());

	Room *room{ PlayState::instance()->getCurrentRoom() };
	if (room != nullptr)
	{
		glm::ivec2 tileXY{ room->getTileCoord({ thisLayer.pos.x, thisLayer.pos.y }) };
		m_layerPosInput[0] = tileXY.x;
		m_layerPosInput[1] = tileXY.y;
	}

	m_layerDepthInput = thisLayer.pos.z;
}

void EditorState::setRoomLayerPos(Room *room, glm::ivec2 pos)
{
	RoomData::Layer &thisLayer{ m_roomLayers[m_selectedLayerId] };
	glm::vec2 roomPos{ room->getTilePos(pos) };
	thisLayer.pos.x = roomPos.x;
	thisLayer.pos.y = roomPos.y;
	room->setLayerPos(m_selectedLayerId, roomPos);
}

void EditorState::updateRoom()
{
	PlayState *playState{ PlayState::instance() };
	Room *room{ playState->getCurrentRoom() };
	if (room != nullptr)
	{
		room->setTiles(m_tiles);
		room->setTilesTexture(m_tilesTexture);
		room->setLayout(m_roomLayout);
	}
}