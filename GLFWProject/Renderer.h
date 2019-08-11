#pragma once
#ifndef Renderer_H
#define Renderer_H

#include <glm/glm.hpp>

class Camera;

class Renderer
{
public:
	// Update the view matrix and window size.
	static void update(const glm::mat4 &viewMatrix, const glm::ivec2 &windowSize);

	// Get the projection matrix, given a zoom value.
	// Larger zoom = more zoomed in.
	static glm::mat4 getProjectionMatrix(float zoom);

	// Reset the buffer data.
	// This should be called every frame from the update loop, so that
	// the proper data can be recalculated.
	virtual void resetData() = 0;

protected:
	// Hold the camera's view matrix.
	static glm::mat4 m_viewMatrix;

	// Hold the current window size.
	static glm::ivec2 m_windowSize;
};

#endif