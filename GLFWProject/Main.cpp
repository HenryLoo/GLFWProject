#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>

#include "GameEngine.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "SpriteRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main()
{
	std::unique_ptr<GameEngine> game = std::make_unique<GameEngine>();
	std::unique_ptr<SpriteRenderer> renderer = std::make_unique<SpriteRenderer>();

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Start the game loop.
	game->start(renderer.get());

	// Game has ended.
	return 0;
}