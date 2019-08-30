#pragma once
#ifndef InputManager_H
#define InputManager_H

#include <GLFW/glfw3.h>

#include <vector>

class InputManager
{
public:
	enum InputType
	{
		INPUT_UP,
		INPUT_DOWN,
		INPUT_LEFT,
		INPUT_RIGHT,
		INPUT_CANCEL,
		INPUT_ATTACK,
		INPUT_JUMP,
		INPUT_EVADE,
		INPUT_SKILL1,
		INPUT_DEBUG,
		INPUT_DEBUG2,
		INPUT_DEBUG3,
		NUM_INPUT_TYPES
	};

	struct InputState
	{
		// Set to true if the key is pressed, else false.
		bool isPressed;

		// Set to true if the key is released, else false.
		bool isReleased;

		// Remaining time in seconds for this input.
		// This is set when the input is initially pressed so that it
		// stays registered as being "pressed" for a short period.
		float duration;
	};

	InputManager(GLFWwindow *window);
	~InputManager();

	void update(float deltaTime);

	// Check key states for an input type.
	// Use the isResetDuration flag if there are multiple instances of checking
	// for that input and the time between the checks are instant.
	bool isKeyPressing(InputType type) const;
	bool isKeyPressed(InputType type, bool isResetDuration = false) const;
	bool isKeyReleased(InputType type) const;

	// Reset the duration for a given input type.
	void resetDuration(InputType type) const;

private:
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void setKey(InputType type, bool isPressed, bool isReleased);

	// Hold input states of each input type.
	// Use InputType as the index, and GLFW input states as the value.
	static std::vector<InputState> m_inputStates;
};

#endif