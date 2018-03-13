#include "StateMachine.hpp"

unsigned int const StateMachine::NoOp = std::numeric_limits<unsigned int>::max();

void StateMachine::noOp(StateMachine&)
{
}

StateMachine::StateMachine()
{
	m_states[NoOp] = &StateMachine::noOp;
	m_currentIndex = NoOp;
}

void StateMachine::addState(unsigned int index, State&& state)
{
	m_states[index] = std::move(state);
}

void StateMachine::addStateBegin(unsigned index, Notification&& state)
{
	m_begins[index] = std::move(state);
}

void StateMachine::addStateEnd(unsigned index, Notification&& state)
{
	m_ends[index] = std::move(state);
}

void StateMachine::change(unsigned int index)
{
	auto const it = m_states.find(index);

	assert (it != m_states.end());

	auto const beginIt = m_begins.find(index);
	auto const endIt = m_ends.find(m_currentIndex);

	if (endIt != m_ends.end())
	{
		endIt->second();
	}
	m_currentState = it->second;
    m_currentIndex = index;
	if (beginIt != m_begins.end())
	{
		beginIt->second();
	}
}

void StateMachine::step()
{
	m_currentState(*this);
}
