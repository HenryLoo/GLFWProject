#pragma once
#ifndef Renderer_H
#define Renderer_H

#include <glad/glad.h>

#include "Shader.h"

#include <memory>
#include <vector>


class Mesh;

class Renderer
{
public:
	Renderer();
	~Renderer();

	// Render all queued meshes.
	void render();

	// Queue an mesh to be rendered.
	void addMesh(Mesh *mesh);

	// Remove all queued meshes.
	void clearMeshes();

private:
	// Hold all queued meshes.
	std::vector<Mesh *> m_meshes;

	// Shaders to render with.
	std::unique_ptr<Shader> m_defaultShader;
};

#endif