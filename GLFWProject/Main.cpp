#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>

#include "GameEngine.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

	//std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(vertices, indices, textures);
	//mesh->setRotation({ 0.f, 0.f, 0.f });
	//mesh->setScale({ 1.f, 1.f, 1.f });
	//mesh->setTranslation({ 0.f, 0.f, 0.f });
	//renderer->addMesh(mesh.get());

	//std::unique_ptr<Mesh> mesh2 = std::make_unique<Mesh>(vertices, indices, textures);
	//mesh2->setRotation({ 0.f, 0.f, 0.f });
	//mesh2->setScale({ 1.f, 1.f, 1.f });
	//mesh2->setTranslation({ 0.5f, 0.f, 0.5f });
	//renderer->addMesh(mesh2.get());

	unsigned int amount = 2000;
	float offset = 2.5f;
	std::vector<std::unique_ptr<Mesh>> meshes;
	meshes.reserve(amount);
	std::vector<glm::mat4> models;
	for (unsigned int i = 0; i < amount; i++)
	{
		meshes.push_back(std::make_unique<Mesh>(vertices, indices, textures));

		meshes[i]->setRotation({ 0.f, 0.f, 0.f });

		float scale = (rand() % 20) / 100.0f + 0.05;
		meshes[i]->setScale({ scale, scale, scale });

		float x = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		meshes[i]->setTranslation({ x, y, z });

		renderer->addMesh(meshes[i].get());

		models.push_back(meshes[i]->getModelMatrix());
	}

	// TODO: find a way to update model matrices each frame, without slow down.
	// Right now, all model matrices are set once at the beginning.
	renderer->models = models;

	// Exit if the game engine could not be created.
	if (game == nullptr)
		return -1;

	// Start the game loop.
	game->start(renderer.get());

	// Game has ended.
	return 0;
}