#include "StateMachine.h"

void StateMachine::update()
{
	if (m_currentState == nullptr)
		return;

	// Edge is defined by {state label, condition function}.
	for (const Edge &e : *m_currentState)
	{
		// Call the edge's condition function to see if it has been satisfied.
		bool isTraversing{ e.condition() };
		if (isTraversing)
		{
			// Move to new state if it exists.
			auto it{ m_states.find(e.destLabel) };
			if (it == m_states.end())
				continue;

			m_currentLabel = it->first;
			m_currentState = &(it->second);

			// Call the update action.
			e.updateAction();

			// Stop checking edges.
			break;
		}
	}
}

const std::string &StateMachine::getState() const
{
	return m_currentLabel;
}

void StateMachine::addState(const std::string &label)
{
	// Add an empty state.
	// Nothing happens if the state already exists.
	std::pair<std::unordered_map<std::string, State>::iterator, bool> result;
	result = m_states.insert(std::pair<std::string, State>(label, {}));

	// If this is the first state added, then set it
	// as the current state.
	if (m_currentLabel.empty() && result.second)
	{
		m_currentLabel = label;
		m_currentState = &(result.first->second);
	}
}

void StateMachine::addEdge(const std::string &srcLabel, 
	const std::string &destLabel, std::function<bool()> condition)
{
	addEdge(srcLabel, destLabel, condition, []() {});
}

void StateMachine::addEdge(const std::string &srcLabel,
	const std::string &destLabel, std::function<bool()> condition,
	std::function<void()> updateAction)
{
	auto it{ m_states.find(srcLabel) };

	// State doesn't exist, so just return.
	if (it == m_states.end())
		return;

	// Otherwise, insert the edge to the state.
	it->second.push_back({ destLabel, condition, updateAction });
}