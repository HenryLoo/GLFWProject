#include "Renderer.h"
#include "Mesh.h"

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

void Renderer::render()
{
	// Clear the colour buffer.
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render each mesh.
	m_defaultShader->use();
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