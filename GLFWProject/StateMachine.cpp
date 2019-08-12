#include "StateMachine.h"

void StateMachine::update()
{
	if (m_currentState == nullptr)
		return;

	// If this is a new state, call the enter action.
	if (isNewState)
	{
		m_currentState->enterAction();
		isNewState = false;
	}

	// Edge is defined by {state label, condition function}.
	for (const Edge &e : m_currentState->edges)
	{
		// Call the edge's condition function to see if it has been satisfied.
		bool isTraversing{ e.condition() };
		if (isTraversing)
		{
			// Move to new state if it exists.
			auto it{ m_states.find(e.destLabel) };
			if (it == m_states.end())
				continue;

			// Call exit action.
			m_currentState->exitAction();

			// Change to new state.
			// The enter action will be called in the next frame,
			// so that player inputs don't stack.
			m_currentLabel = it->first;
			m_currentState = &(it->second);
			isNewState = true;

			// Stop checking edges.
			break;
		}
	}

	// Call update action.
	m_currentState->updateAction();
}

const std::string &StateMachine::getState() const
{
	return m_currentLabel;
}

void StateMachine::addState(const std::string &label,
	std::function<void()> updateAction,
	std::function<void()> enterAction,
	std::function<void()> exitAction)
{
	// Add an empty state.
	// Nothing happens if the state already exists.
	std::pair<std::unordered_map<std::string, State>::iterator, bool> result;
	result = m_states.insert(std::pair<std::string, State>(label, 
		{ {}, updateAction, enterAction, exitAction }));

	// If this is the first state added, then set it
	// as the current state.
	if (m_currentLabel.empty() && result.second)
	{
		m_currentLabel = label;
		m_currentState = &(result.first->second);
		m_currentState->updateAction = updateAction;
		m_currentState->enterAction = enterAction;
		m_currentState->exitAction = exitAction;
	}
}

void StateMachine::addEdge(const std::string &srcLabel,
	const std::string &destLabel, std::function<bool()> condition)
{
	auto it{ m_states.find(srcLabel) };

	// State doesn't exist, so just return.
	if (it == m_states.end())
		return;

	// Otherwise, insert the edge to the state.
	it->second.edges.push_back({ destLabel, condition });
}