#pragma once
#ifndef InputManager_H
#define InputManager_H

#include <GLFW/glfw3.h>

#include <vector>

enum InputType
{
	INPUT_UP,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_RIGHT,
	INPUT_CANCEL,
	INPUT_ATTACK,
	INPUT_JUMP,
	INPUT_DEBUG,
	NUM_INPUT_TYPES
};

class InputManager
{
public:
	InputManager();
	~InputManager();

	// Poll inputs. This should be called once every game loop iteration.
	void processInput(GLFWwindow *window);

	// Check key states for an input type.
	bool isKeyPressing(InputType type);
	bool isKeyReleased(InputType type);
	bool isKeyPressed(InputType type);

private:
	// Hold input states of each input type.
	// Use InputType as the index, and GLFW input states as the value.
	std::vector<int> m_inputStates;
	std::vector<int> m_prevInputStates;
};

#endif