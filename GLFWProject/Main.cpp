#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "GameEngine.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"

int main()
{
	std::unique_ptr<GameEngine> game = std::make_unique<GameEngine>();
	std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();

	// TODO: remove this later.
	std::vector<Vertex> vertices = {
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // top right
		{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}} // top left
	};

	std::vector<GLuint> indices = {
		0, 1, 3, // first triangle
		1, 2, 3 // second triangle
	};

	std::unique_ptr<Texture> texture = std::make_unique<Texture>("serah_idle.png");
	std::vector<Texture *> textures = { texture.get() };

	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(vertices, indices, textures);
	renderer->addMesh(mesh.get());

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Start the game loop.
	game->start(renderer.get());

	// Game has ended.
	return 0;
}