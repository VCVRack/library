#if!defined STATEMACHINE_HPP
#define STATEMACHINE_HPP
#include <memory>
#include <map>
#include <limits>
#include <functional>

#include <rack.hpp>

class StateMachine
{
public:
	using State = std::function<void(StateMachine&)>;
	using Notification = std::function<void()>;
	static unsigned int const NoOp;

	static void noOp(StateMachine&);

	StateMachine();

	void addState(unsigned int index, State&& state);
	void addStateBegin(unsigned index, Notification&& state);
	void addStateEnd(unsigned index, Notification&& state);
	void change(unsigned int index);

	void step();

	unsigned int currentIndex()const
	{
		return m_currentIndex;
	}
private:
	std::map<unsigned int, State> m_states;
	std::map<unsigned int, Notification> m_begins;
	std::map<unsigned int, Notification> m_ends;
	State m_currentState;
	unsigned int m_currentIndex;
};

#endif
