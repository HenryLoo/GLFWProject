#include "InputManager.h"

InputManager::InputManager()
{
	for (int i = 0; i < NUM_INPUT_TYPES; ++i)
	{
		m_inputStates.push_back(GLFW_RELEASE);
	}
	m_prevInputStates = m_inputStates;
}

InputManager::~InputManager()
{

}

void InputManager::processInput(GLFWwindow *window)
{
	// Store the inputs from the previous frame.
	for (int i = 0; i < NUM_INPUT_TYPES; ++i)
	{
		m_prevInputStates[i] = m_inputStates[i];
	}

	// Poll IO events.
	glfwPollEvents();

	m_inputStates[INPUT_UP] = glfwGetKey(window, GLFW_KEY_UP);
	m_inputStates[INPUT_DOWN] = glfwGetKey(window, GLFW_KEY_DOWN);
	m_inputStates[INPUT_LEFT] = glfwGetKey(window, GLFW_KEY_LEFT);
	m_inputStates[INPUT_RIGHT] = glfwGetKey(window, GLFW_KEY_RIGHT);
	m_inputStates[INPUT_CANCEL] = glfwGetKey(window, GLFW_KEY_ESCAPE);
	m_inputStates[INPUT_ATTACK] = glfwGetKey(window, GLFW_KEY_X);
	m_inputStates[INPUT_JUMP] = glfwGetKey(window, GLFW_KEY_C);
	m_inputStates[INPUT_DEBUG] = glfwGetKey(window, GLFW_KEY_F1);
}

bool InputManager::isKeyPressing(InputType type)
{
	return m_inputStates[type] == GLFW_PRESS;
}

bool InputManager::isKeyReleased(InputType type)
{
	return m_inputStates[type] == GLFW_RELEASE;
}

bool InputManager::isKeyPressed(InputType type)
{
	return m_inputStates[type] == GLFW_PRESS && m_prevInputStates[type] == GLFW_RELEASE;
}