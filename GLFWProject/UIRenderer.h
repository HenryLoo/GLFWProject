#pragma once
#ifndef UIRenderer_H
#define UIRenderer_H

#include "GameComponent.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <memory>

class Camera;
class GameEngine;

class UIRenderer
{
public:
	UIRenderer(GameEngine &game);
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

	void render(Camera *camera, glm::ivec2 windowSize);

private:
	// Shaders to render with.
	std::shared_ptr<Shader> m_boxShader;

	// Data to send to the GPU.
	GLfloat *m_posSizeData;
	GLubyte *m_colourData;

	// The number of boxes to render for the current frame.
	int m_numBoxes{ 0 };

	// The vertex array object and vertex buffer object for instances.
	GLuint m_VAO, m_verticesVBO, m_posSizeVBO, m_colourVBO;
};

#endif