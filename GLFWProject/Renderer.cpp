#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Renderer::Renderer()
{
	// Render as wireframe.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Enable blending for transparency in textures.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load the shader.
	m_defaultShader = std::make_unique<Shader>("default.vs", "default.fs");
}

Renderer::~Renderer()
{

}

void Renderer::render(Camera *camera, float aspectRatio)
{
	// Clear the colour buffer.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render each mesh.
	m_defaultShader->use();

	// Set the view matrix uniform.
	m_defaultShader->setMat4("view", camera->getViewMatrix());

	// Set projection matrix uniform.
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f),
		aspectRatio, 0.1f, 100.0f);
	m_defaultShader->setMat4("projection", projMatrix);

	for (Mesh *mesh : m_meshes)
	{
		mesh->render(m_defaultShader.get());
	}
}

void Renderer::addMesh(Mesh *mesh)
{
	// Store the mesh to render later.
	m_meshes.push_back(mesh);
}

void Renderer::clearMeshes()
{
	m_meshes.clear();
}