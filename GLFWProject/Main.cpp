#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>

#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "InputManager.h"
#include "UIRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main()
{
	std::unique_ptr<GameEngine> game = std::make_unique<GameEngine>();
	std::unique_ptr<SpriteRenderer> sRenderer = std::make_unique<SpriteRenderer>();
	std::unique_ptr<InputManager> input = std::make_unique<InputManager>();
	std::unique_ptr<UIRenderer> uRenderer = std::make_unique<UIRenderer>();

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Start the game loop.
	game->start(sRenderer.get(), input.get(), uRenderer.get());

	// Game has ended.
	return 0;
}