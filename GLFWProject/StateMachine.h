#pragma once
#ifndef StateMachine_H
#define StateMachine_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct Edge
{
	// The label of the destination state.
	std::string destLabel;

	// Condition to check whether to traverse this edge.
	std::function<bool(int entityId)> condition;
};

struct State
{
	// All the edges connected to this state.
	std::vector<Edge> edges;

	// Function to call at update.
	std::function<void(int entityId)> updateAction;

	// Function to call when the state machine enters this state.
	std::function<void(int entityId)> enterAction;

	// Function to call when the state machine leaves this state.
	std::function<void(int entityId)> exitAction;
};

class StateMachine
{
public:
	// Process the current state.
	void update(int entityId);

	// Get the current's label.
	const std::string &getState() const;

	// Add a state to the machine.
	void addState(const std::string &label,
		std::function<void(int)> updateAction = [](int) {},
		std::function<void(int)> enterAction = [](int) {},
		std::function<void(int)> exitAction = [](int) {});

	// Add a transition edge between two existing states in the machine.
	void addEdge(const std::string &srcLabel, const std::string &destLabel,
		std::function<bool(int)> condition);

private:
	// Map state labels to their respective states.
	std::unordered_map<std::string, State> m_states;

	// Hold the current state and its label.
	std::string m_currentLabel;
	State *m_currentState{ nullptr };

	// Flag for if this is the first frame since entering the current state.
	bool isNewState{ false };
};

#endif