#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

// Static variables.
glm::mat4 Renderer::m_viewMatrix;
glm::ivec2 Renderer::m_windowSize;

void Renderer::updateViewMatrix(const glm::mat4& viewMatrix)
{
	m_viewMatrix = viewMatrix;
}

void Renderer::updateWindowSize(const glm::ivec2 &windowSize)
{
	m_windowSize = windowSize;
}

glm::mat4 Renderer::getOrthographicMatrix(float zoom)
{
	glm::vec2 halfScreenSize{ m_windowSize.x / 2.f, m_windowSize.y / 2.f };
	glm::mat4 projectionMatrix{ glm::ortho(
		-halfScreenSize.x / zoom, halfScreenSize.x / zoom,
		-halfScreenSize.y / zoom, halfScreenSize.y / zoom,
		-1000.0f, 1000.0f) };

	return projectionMatrix;
}

glm::mat4 Renderer::getPerspectiveMatrix(float fovY)
{
	glm::vec2 halfScreenSize{ m_windowSize.x / 2.f, m_windowSize.y / 2.f };
	glm::mat4 projectionMatrix{ glm::perspective(
		glm::radians(fovY), (float)m_windowSize.x / (float)m_windowSize.y, 0.1f, 1000.f
	) };

	return projectionMatrix;
}