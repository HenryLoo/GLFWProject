#pragma once
#ifndef Renderer_H
#define Renderer_H

#include <glad/glad.h>

#include "Shader.h"

#include <memory>
#include <vector>


class Mesh;
class Camera;

class Renderer
{
public:
	Renderer();
	~Renderer();

	// Update all renderer values.
	void update(float deltaTime);

	// Render all queued meshes.
	void render(Camera *camera, float aspectRatio);

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