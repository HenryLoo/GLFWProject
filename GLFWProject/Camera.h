#pragma once
#ifndef Camera_H
#define Camera_H

#include <glm/glm.hpp>

class Camera
{
public:
	enum class Direction
	{
		Forward,
		Backward,
		Left,
		Right
	};

	Camera();

	// Update the camera's values.
	void update(float deltaTime, glm::vec3 playerPos, 
		glm::ivec2 windowSize, glm::ivec2 roomSize);

	// Move the camera in a direction.
	void move(Direction direction);

	// Make the camera look at a position on the screen.
	void lookAt(glm::vec2 screenPos);

	// Get this camera's view matrix.
	glm::mat4 getViewMatrix();

	// Get this camera's position.
	glm::vec3 getPosition() const;

	// Get the camera's zoom.
	float getZoom() const;

	// Get the camera's vertical view of view.
	float getFovY() const;

private:
	// The camera's position.
	glm::vec3 m_position{ 0.f };

	// The vector that points in the positive y-axis in the camera space.
	glm::vec3 m_cameraUp{ 0.f, 1.f, 0.f };

	// The vector that points in the positive z-axis in the camera space.
	glm::vec3 m_cameraFront{ 0.f, 0.f, -1.f };

	// The view matrix that corresponds to this camera.
	glm::mat4 m_viewMatrix{ 1.f };

	// The speed at which the camera moves.
	float m_speed{ 2.5f };

	// The current velocity of the camera.
	glm::vec3 m_velocity{ 0.f };

	// Use this flag to prevent camera jumps on the first frame.
	bool m_isFirstFrame{ true };

	// The previous frame's screen position that the camera was looking at.
	glm::vec2 m_lastLookPos{ 0.f };

	// The look sensitivity of the camera.
	float m_sensitivity{ 0.05f };

	// The camera's rotational values.
	float m_yaw{ -90.0f };
	float m_pitch{ 0.f };

	// The vertical field of view for perspective projection, in degrees.
	float m_fovY{ 45.f };

	// The zoom multiplier. Higher values = more zoomed in.
	float m_zoom{ 4.f };
};

#endif