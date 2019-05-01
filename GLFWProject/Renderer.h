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

	// Create a new framebuffer and its appropriate attachments with 
	// a given screen size.
	void createFramebuffer(int screenWidth, int screenHeight);

	// Hold all queued meshes.
	std::vector<Mesh *> m_meshes;
	std::vector<glm::mat4> models;
private:

	// Shaders to render with.
	std::unique_ptr<Shader> m_defaultShader;
	std::unique_ptr<Shader> m_screenShader;

	// The framebuffer used for post-processing.
	GLuint m_screenVAO, m_screenVBO, m_screenFBO, m_screenColourBuffer, m_screenRBO;

	// TODO: used for instancing test, fix later.
	GLuint m_instanceVBO;
};

#endif