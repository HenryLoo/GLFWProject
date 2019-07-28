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
	std::function<bool()> condition;

	// Function to call when this edge is traversed.
	std::function<void()> updateAction;
};

// A state is defined as a collection of edges.
typedef std::vector<Edge> State;

class StateMachine
{
public:
	// Process the current state.
	void update();

	// Get the current's label.
	const std::string &getState() const;

	// Add a state to the machine.
	void addState(const std::string &label);

	// Add a transition edge between two existing states in the machine.
	void addEdge(const std::string &srcLabel, const std::string &destLabel,
		std::function<bool()> condition);
	void addEdge(const std::string &srcLabel, const std::string &destLabel,
		std::function<bool()> condition, std::function<void()> updateAction);

private:
	// Map state labels to their respective states.
	std::unordered_map<std::string, State> m_states;

	// Hold the current state and its label.
	std::string m_currentLabel;
	State *m_currentState{ nullptr };
};

#endif