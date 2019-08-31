#include "InputManager.h"

#include <iostream>

namespace
{
	// Duration in seconds to allow the press state of an input to linger.
	const float PRESS_DURATION{ 0.1f };

	const int NUM_MOUSE_BUTTONS{ 2 };
}

std::vector<InputManager::InputState> InputManager::m_inputStates;
glm::vec2 InputManager::m_mousePos;
std::vector<int> InputManager::m_mouseStates;

InputManager::InputManager(GLFWwindow *window)
{
	for (int i = 0; i < NUM_INPUT_TYPES; ++i)
	{
		m_inputStates.push_back({ false, true, 0.f });
	}

	glfwSetKeyCallback(window, InputManager::keyCallback);

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		m_mouseStates.push_back(GLFW_RELEASE);
	}
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

InputManager::~InputManager()
{

}

void InputManager::update(float deltaTime)
{
	// Update durations for all input states.
	for (InputState &state : m_inputStates)
	{
		if (state.duration > 0.f)
		{
			state.duration -= deltaTime;
			state.duration = glm::max(0.f, state.duration);
		}
	}
}

bool InputManager::isKeyPressing(InputType type) const
{
	return !m_inputStates[type].isReleased;
}

bool InputManager::isKeyPressed(InputType type, bool isResetDuration) const
{
	// Reset the input to guarantee that it only remains
	// pressed for 1 frame.
	InputState &state{ m_inputStates[type] };
	bool isPressed{ state.isPressed || state.duration > 0.f};
	if (isPressed)
	{
		state.isPressed = false;
		state.isReleased = true;

		if (isResetDuration)
		{
			for (InputState &state : m_inputStates)
			{
				state.duration = 0.f;
			}
		}
	}

	return isPressed;
}

bool InputManager::isKeyReleased(InputType type) const
{
	return m_inputStates[type].isReleased;
}

void InputManager::resetDuration(InputType type) const
{
	m_inputStates[type].duration = 0.f;
}

bool InputManager::isMousePressed(int button) const
{
	return m_mouseStates[button] == GLFW_PRESS;
}

bool InputManager::isMouseReleased(int button) const
{
	return m_mouseStates[button] == GLFW_RELEASE;
}

glm::vec2 InputManager::getMousePos() const
{
	return m_mousePos;
}

void InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	bool isPressed{ action == GLFW_PRESS };
	bool isReleased{ action == GLFW_RELEASE };

	switch (key)
	{
		case GLFW_KEY_UP:
			setKey(INPUT_UP, isPressed, isReleased);
			break;
		case GLFW_KEY_DOWN:
			setKey(INPUT_DOWN, isPressed, isReleased);
			break;
		case GLFW_KEY_LEFT:
			setKey(INPUT_LEFT, isPressed, isReleased);
			break;
		case GLFW_KEY_RIGHT:
			setKey(INPUT_RIGHT, isPressed, isReleased);
			break;
		case GLFW_KEY_ESCAPE:
			setKey(INPUT_CANCEL, isPressed, isReleased);
			break;
		case GLFW_KEY_X:
			setKey(INPUT_ATTACK, isPressed, isReleased);
			break;
		case GLFW_KEY_C:
			setKey(INPUT_JUMP, isPressed, isReleased);
			break;
		case GLFW_KEY_SPACE:
			setKey(INPUT_EVADE, isPressed, isReleased);
			break;
		case GLFW_KEY_Z:
			setKey(INPUT_SKILL1, isPressed, isReleased);
			break;
		case GLFW_KEY_F1:
			setKey(INPUT_DEBUG, isPressed, isReleased);
			break;
		case GLFW_KEY_F2:
			setKey(INPUT_DEBUG2, isPressed, isReleased);
			break;
		case GLFW_KEY_F3:
			setKey(INPUT_DEBUG3, isPressed, isReleased);
			break;
	}
}

void InputManager::setKey(InputType type, bool isPressed, bool isReleased)
{
	m_inputStates[type].isPressed = isPressed;
	m_inputStates[type].isReleased = isReleased;

	// Set duration on press so that it lingers on the "pressed" state
	// for a bit.
	if (isPressed)
		m_inputStates[type].duration = PRESS_DURATION;
}

void InputManager::mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
	m_mousePos.x = static_cast<float>(xpos);
	m_mousePos.y = static_cast<float>(ypos);
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	m_mouseStates[button] = action;
}