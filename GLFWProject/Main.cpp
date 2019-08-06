#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>

#include "GameEngine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>

int main()
{
	std::unique_ptr<GameEngine> game = std::make_unique<GameEngine>();

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Seed the random number generator.
	srand(static_cast<unsigned> (time(0)));

	// Start the game loop.
	game->start();

	// Game has ended.
	return 0;
}