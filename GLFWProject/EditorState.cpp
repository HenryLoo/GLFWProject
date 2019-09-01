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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

	Room *room{ PlayState::instance()->getCurrentRoom() };
	if (room != nullptr)
	{
		const RoomData::Data &roomData{ room->getRoomData() };
		strcpy_s(m_nameInput, sizeof(m_nameInput), roomData.name.c_str());
		strcpy_s(m_tilesInput, sizeof(m_tilesInput), roomData.tilesName.c_str());
		strcpy_s(m_bgTextureInput, sizeof(m_bgTextureInput), roomData.bgTextureName.c_str());
		strcpy_s(m_musicInput, sizeof(m_musicInput), roomData.musicName.c_str());
		strcpy_s(m_shaderInput, sizeof(m_shaderInput), roomData.shaderName.c_str());

		m_tilesTexture = assetLoader->load<Texture>(roomData.tilesName);

		// Set up room layout texture.
		m_roomLayout = roomData.layout;
		glm::ivec2 roomSize{ room->getSize() };
		GLubyte *layoutData{ new GLubyte[roomSize.x * roomSize.y * 4] };
		for (int i = 0; i < roomSize.y; ++i)
		{
			for (int j = 0; j < roomSize.x; ++j)
			{
				GLubyte r{ 255 }, g{ 255 }, b{ 255 }, a{ 255 };
				int layoutIndex{ roomSize.x * i + j };
				if (m_roomLayout[layoutIndex] != RoomData::TILE_SPACE)
				{
					r = (GLubyte)m_roomLayout[layoutIndex] - 1;
					g = b = 0;
				}

				// Image is vertically flipped.
				int firstIndex{ 4 * (roomSize.x * roomSize.y - roomSize.x * (i + 1) + j)};
				layoutData[firstIndex] = r;
				layoutData[firstIndex + 1] = g;
				layoutData[firstIndex + 2] = b;
				layoutData[firstIndex + 3] = a;
			}
		}

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, roomSize.x, roomSize.y, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, layoutData);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_layoutTexture = std::make_shared<Texture>(textureId, roomSize.x, roomSize.y, 4);

		delete[] layoutData;

		// Show layout texture if the current menu is the layout selector.
		if (m_currentMenu == MENU_LAYOUT)
			room->setTileSprites(m_layoutTexture);
	}
}

void EditorState::cleanUp()
{
	if (m_tilesTexture != nullptr)
	{
		Room *room{ PlayState::instance()->getCurrentRoom() };
		if (room != nullptr)
		{
			room->setTileSprites(m_tilesTexture);
			room->setLayout(m_roomLayout);
		}
	}
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
}

void EditorState::update(float deltaTime, const glm::ivec2 &windowSize,
	EntityManager *entityManager, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer, SoLoud::Soloud &soundEngine)
{
	createUI();
	placeTile(windowSize);
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
		sRenderer->render(camera, room);
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

		std::ofstream o(OUTPUT_PATH + "room.json");
		o << json << std::endl;
		o.close();

		// Write the room's tiles texture.
		Texture *texture{ room->getTileSprites() };
		if (texture != nullptr)
		{
			texture->bind();
			glm::ivec2 size{ texture->getSize() };
			int numChannels{ texture->getNumChannels() };

			stbi_uc *data{ new stbi_uc[size.x * size.y * numChannels] };
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_flip_vertically_on_write(true);
			std::string output{ OUTPUT_PATH + "room.png" };
			stbi_write_png(output.c_str(), size.x, size.y, numChannels, data, 0);

			delete[] data;
		}
		
	}
}

void EditorState::placeTile(const glm::ivec2 &windowSize)
{
	if (m_currentMenu != MENU_TILES && m_currentMenu != MENU_LAYOUT)
		return;

	m_prevClickedTile = m_clickedTile;

	// Don't allow clicking through the editor UI.
	if (m_isLeftClicked &&
		!ImGui::IsAnyItemHovered() && !ImGui::IsAnyWindowHovered())
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
				m_prevTileToPlace = m_tileToPlace;

				// Change the pixel on the tiles texture.
				if (m_tilesTexture != nullptr)
				{
					char pixel[4]{ (char)m_tileToPlace.x, (char)m_tileToPlace.y, 
						(char)0, (char)255 };
					m_tilesTexture->bind();
					glTexSubImage2D(GL_TEXTURE_2D, 0, m_clickedTile.x,
						m_clickedTile.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
				}
			}
			// Placing tile type with layout selector.
			else if (m_currentMenu == MENU_LAYOUT && (m_prevClickedTile != m_clickedTile ||
				m_prevTypeToPlace != m_typeToPlace))
			{
				m_prevTypeToPlace = m_typeToPlace;

				// Change the pixel on the layout texture.
				if (m_layoutTexture != nullptr)
				{
					char pixel[4]{ (char)m_typeToPlace.x, (char)m_typeToPlace.y,
						(char)0, (char)255 };
					m_layoutTexture->bind();
					glTexSubImage2D(GL_TEXTURE_2D, 0, m_clickedTile.x,
						m_clickedTile.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
				}

				// Change the layout vector.
				RoomData::TileType type{ RoomData::TILE_SPACE };
				if (m_typeToPlace != ERASER_TILE)
					type = static_cast<RoomData::TileType>(m_typeToPlace.x + 1);
				int layoutIndex{ room->getTileIndex(m_clickedTile) };
				m_roomLayout[layoutIndex] = type;
			}
		}
	}
}

void EditorState::createUI()
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
			if (ImGui::MenuItem("Save")) { save(); }
			if (ImGui::MenuItem("Reload Room")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Properties")) { m_currentMenu = MENU_PROPERTIES; }
			if (ImGui::MenuItem("Tiles")) 
			{ 
				m_currentMenu = MENU_TILES; 
				if (hasRoom && m_tilesTexture != nullptr)
					room->setTileSprites(m_tilesTexture);
			}
			if (ImGui::MenuItem("Layers")) { m_currentMenu = MENU_LAYERS; }
			if (ImGui::MenuItem("Layout"))
			{
				m_currentMenu = MENU_LAYOUT;
				if (hasRoom && m_layoutTexture != nullptr)
					room->setTileSprites(m_layoutTexture);
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
			ImGui::InputText("Tiles", m_tilesInput, IM_ARRAYSIZE(m_tilesInput));
			ImGui::InputText("Background", m_bgTextureInput, IM_ARRAYSIZE(m_bgTextureInput));
			ImGui::InputText("Music", m_musicInput, IM_ARRAYSIZE(m_musicInput));
			ImGui::InputText("Shader", m_shaderInput, IM_ARRAYSIZE(m_shaderInput));

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
	}

	ImGui::End();

	ImGui::EndFrame();
}