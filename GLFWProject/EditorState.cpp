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

#include <json/single_include/nlohmann/json.hpp>

#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <iomanip>

namespace
{
	const std::string OUTPUT_PATH{ "editor/" };
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
}

void EditorState::cleanUp()
{

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

	// Save the room.
	if (inputManager->isKeyPressed(InputManager::INPUT_DEBUG3, true))
	{
		save();
	}
}

void EditorState::update(float deltaTime, const glm::ivec2 &windowSize,
	EntityManager *entityManager, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer, UIRenderer *uRenderer,
	TextRenderer *tRenderer, SoLoud::Soloud &soundEngine)
{
	// Clear the renderer data, since updating may repopulate these.
	tRenderer->resetData();

	tRenderer->addText("ROOM EDITOR", m_font.get(),
		glm::vec2(16.f, windowSize.y - 32.f));
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

	// Render queued text.
	tRenderer->render();
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