#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>

#include "GameEngine.h"
#include "EntityManager.h"
#include "SpriteRenderer.h"
#include "UIRenderer.h"
#include "InputManager.h"
#include "TextRenderer.h"

#include "SpriteSheet.h"
#include "Shader.h"
#include "Sound.h"
#include "Prefab.h"
#include "Font.h"

#include "AssetLoader.h"
#include "DiskStream.h"
#include "TextureLoader.h"
#include "SpriteSheetLoader.h"
#include "RoomLoader.h"
#include "ShaderLoader.h"
#include "SoundLoader.h"
#include "PrefabLoader.h"
#include "FontLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wav.h>

#include <cstdlib>
#include <ctime>

int main()
{
	std::unique_ptr<GameEngine> game{ std::make_unique<GameEngine>() };

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Initialize the asset loader.
	std::unique_ptr<AssetLoader> assetLoader;
	assetLoader = std::make_unique<AssetLoader>(new DiskStream());
	assetLoader->registerLoader<Texture>(new TextureLoader());
	assetLoader->registerLoader<SpriteSheet>(new SpriteSheetLoader());
	assetLoader->registerLoader<Room>(new RoomLoader());
	assetLoader->registerLoader<Shader>(new ShaderLoader());
	assetLoader->registerLoader<Sound>(new SoundLoader());
	assetLoader->registerLoader<Prefab>(new PrefabLoader());
	assetLoader->registerLoader<Font>(new FontLoader());

	std::unique_ptr<SpriteRenderer> sRenderer{ 
		std::make_unique<SpriteRenderer>(assetLoader.get()) };
	std::unique_ptr<UIRenderer> uRenderer{ 
		std::make_unique<UIRenderer>(assetLoader.get()) };
	std::unique_ptr<InputManager> inputManager{ 
		std::make_unique<InputManager>(game->getWindow()) };
	std::unique_ptr<TextRenderer> tRenderer{
		std::make_unique<TextRenderer>(assetLoader.get()) };

	// Initialize the sound engine.
	SoLoud::Soloud soundEngine;
	soundEngine.init();

	std::unique_ptr<EntityManager> entityManager{ 
		std::make_unique<EntityManager>( game.get(), assetLoader.get(), 
		inputManager.get(), sRenderer.get(), uRenderer.get(),
		soundEngine) };

	// Seed the random number generator.
	srand(static_cast<unsigned> (time(0)));

	// Start the game loop.
	game->start(entityManager.get(), assetLoader.get(), inputManager.get(),
		sRenderer.get(), uRenderer.get(), tRenderer.get());

	// Deinitialize the sound engine before closing.
	soundEngine.deinit();

	// Game has ended.
	return 0;
}