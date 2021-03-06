#include "GameEngine.h"
#include "EntityManager.h"
#include "SpriteRenderer.h"
#include "UIRenderer.h"
#include "InputManager.h"
#include "TextRenderer.h"

#include "PlayState.h"

#include "SpriteSheet.h"
#include "Shader.h"
#include "Sound.h"
#include "Prefab.h"
#include "Music.h"
#include "Font.h"
#include "Script.h"

#include "AssetLoader.h"
#include "DiskStream.h"
#include "TextureLoader.h"
#include "SpriteSheetLoader.h"
#include "ShaderLoader.h"
#include "SoundLoader.h"
#include "PrefabLoader.h"
#include "MusicLoader.h"
#include "FontLoader.h"
#include "ScriptLoader.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wav.h>

#include <iostream>

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
	assetLoader->registerLoader<Shader>(new ShaderLoader());
	assetLoader->registerLoader<Sound>(new SoundLoader());
	assetLoader->registerLoader<Prefab>(new PrefabLoader());
	assetLoader->registerLoader<Music>(new MusicLoader());
	assetLoader->registerLoader<Font>(new FontLoader());
	assetLoader->registerLoader<Script>(new ScriptLoader());

	std::unique_ptr<SpriteRenderer> sRenderer{
		std::make_unique<SpriteRenderer>(assetLoader.get()) };
	std::unique_ptr<UIRenderer> uRenderer{
		std::make_unique<UIRenderer>(assetLoader.get()) };
	std::unique_ptr<InputManager> inputManager{
		std::make_unique<InputManager>(game->getWindow()) };
	std::unique_ptr<TextRenderer> tRenderer{
		std::make_unique<TextRenderer>(assetLoader.get()) };

	// Initialize the sound engine.
	SoLoud::Soloud *soundEngine{ new SoLoud::Soloud() };
	soundEngine->init();

	std::unique_ptr<EntityManager> entityManager{
		std::make_unique<EntityManager>(game.get(), assetLoader.get(),
		inputManager.get(), sRenderer.get(), uRenderer.get(),
		*soundEngine) };

	// Setup ImGui after input manager to properly install key callbacks.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(game->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");

	// Seed the random number generator.
	srand(static_cast<unsigned> (time(0)));

	// Start the game loop.
	game->pushState(PlayState::instance(), assetLoader.get());
	game->start(entityManager.get(), assetLoader.get(), inputManager.get(),
		sRenderer.get(), uRenderer.get(), tRenderer.get(), *soundEngine);

	// Deinitialize the sound engine before closing.
	soundEngine->deinit();

	// Clean up ImGui.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Game has ended.
	return 0;
}