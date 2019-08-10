#pragma once
#ifndef UIRenderer_H
#define UIRenderer_H

#include "GameComponent.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

class Camera;
class Shader;
class AssetLoader;

class UIRenderer
{
public:
	UIRenderer(AssetLoader *assetLoader);
	~UIRenderer();

	// Add box data to the array of boxes to prepare for rendering.
	// The rgba colour values range from 0-255.
	void addBox(const GameComponent::Physics &physics, 
		const AABB &aabb,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	// Set the number of boxes to render to 0.
	// This should be called every frame from the update loop, so that
	// the proper number of boxes can be recalculated.
	void resetNumBoxes();

	// Update the view matrix.
	void update(const glm::mat4& viewMatrix);

	// Render the HUD and all queued boxes.
	void render(Camera *camera, glm::ivec2 windowSize);

private:
	// Shaders to render with.
	std::shared_ptr<Shader> m_boxShader;

	// Data to send to the GPU.
	std::vector<GLubyte> m_colourData;
	std::vector<glm::mat4> m_modelViewsData;

	// The vertex array object and vertex buffer object for instances.
	GLuint m_VAO, m_verticesVBO, m_colourVBO, m_modelViewsVBO;

	// Hold the camera's view matrix.
	glm::mat4 m_viewMatrix;
};

#endif