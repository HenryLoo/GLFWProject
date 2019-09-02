#include "Camera.h"

#include "Room.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Camera::Camera()
{

}

void Camera::update(float deltaTime, glm::vec3 pos, 
	glm::ivec2 windowSize, glm::ivec2 roomSize, bool isInstant)
{
	glm::ivec2 windowHalfSize{ windowSize / 2 };
	glm::ivec2 roomSizePixel{ roomSize * Room::TILE_SIZE };

	// Set depth distance of camera.
	m_position.z = (windowHalfSize.y / m_zoom) / glm::tan(glm::radians(m_fovY / 2));

	// Move the camera towards the player.
	// The velocity is faster if further away from the target position.
	if (!isInstant)
	{
		glm::vec2 distance{ pos.x - m_position.x, pos.y - m_position.y };
		glm::vec2 velocity{ distance / 0.25f };
		m_position.x += velocity.x * deltaTime;
		m_position.y += velocity.y * deltaTime;
	}
	else
	{
		m_position.x = pos.x;
		m_position.y = pos.y;
	}

	// Keep the camera within the room bounds.
	m_position.x = glm::clamp(m_position.x,
		windowHalfSize.x / m_zoom,
		roomSizePixel.x - windowHalfSize.x / m_zoom);
	m_position.y = glm::clamp(m_position.y,
		windowHalfSize.y / m_zoom,
		roomSizePixel.y - windowHalfSize.y / m_zoom);

	// Update the camera's position by its current velocity.
	//m_position += m_velocity * deltaTime;

	// Reset the velocity so that the camera only moves on input.
	//m_velocity = glm::vec3(0.f);
}

void Camera::move(Direction direction)
{
	glm::vec3 velocity = glm::vec3(0.f);

	switch (direction)
	{
		case Direction::Forward:
			velocity = m_cameraFront * m_speed;
			break;
		case Direction::Backward:
			velocity = -m_cameraFront * m_speed;
			break;
		case Direction::Left:
			velocity = -glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * m_speed;
			break;
		case Direction::Right:
			velocity = glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * m_speed;
			break;
	}

	// Set the velocity if it is currently 0, otherwise add onto it.
	(m_velocity == glm::vec3(0.f)) ? m_velocity = velocity : m_velocity += velocity;
}

void Camera::lookAt(glm::vec2 screenPos)
{
	if (m_isFirstFrame)
	{
		m_lastLookPos = screenPos;
		m_isFirstFrame = false;
	}

	glm::vec2 offset = glm::vec2(screenPos.x - m_lastLookPos.x, m_lastLookPos.y - screenPos.y);
	m_lastLookPos = screenPos;

	offset *= m_sensitivity;
	m_yaw += offset.x;
	m_pitch += offset.y;
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_cameraFront = glm::normalize(front);
}

glm::mat4 Camera::getViewMatrix()
{
	// Calculate the LookAt matrix.
	m_viewMatrix = glm::lookAt(
		m_position,
		m_position + m_cameraFront,
		m_cameraUp);

	return m_viewMatrix;
}

glm::vec3 Camera::getPosition() const
{
	return m_position;
}

float Camera::getZoom() const
{
	return m_zoom;
}

float Camera::getFovY() const
{
	return m_fovY;
}