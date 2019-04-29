#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "GameEngine.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"

int main()
{
	std::unique_ptr<GameEngine> game = std::make_unique<GameEngine>();
	std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();

	// TODO: remove this later.
	std::vector<Vertex> vertices = {
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // top right
		{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // bottom right
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom left
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}} // top left
	};

	std::vector<GLuint> indices = {
		0, 1, 3, // first triangle
		1, 2, 3 // second triangle
	};

	// Use the same texture for both diffuse and specular maps.
	std::unique_ptr<Texture> texture = std::make_unique<Texture>("serah_idle.png");
	std::vector<Texture *> textures = { texture.get(), texture.get() };

	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(vertices, indices, textures);
	mesh->setRotation({ 0.f, 0.f, 0.f });
	mesh->setScale({ 1.f, 1.f, 1.f });
	mesh->setTranslation({ 0.f, 0.f, 0.f });
	renderer->addMesh(mesh.get());

	std::unique_ptr<Mesh> mesh2 = std::make_unique<Mesh>(vertices, indices, textures);
	mesh2->setRotation({ 0.f, 0.f, 0.f });
	mesh2->setScale({ 1.f, 1.f, 1.f });
	mesh2->setTranslation({ 0.5f, 0.f, 0.5f });
	renderer->addMesh(mesh2.get());

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Start the game loop.
	game->start(renderer.get());

	// Game has ended.
	return 0;
}