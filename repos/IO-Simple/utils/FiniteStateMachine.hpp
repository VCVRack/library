#if!defined FINITESTATEMACHINE_HPP
#define FINITESTATEMACHINE_HPP
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <functional>
#include <memory>

template <class K, class ... A>
class FiniteStateMachine
{
public:
	class AState
	{
	protected:
		explicit AState(std::string&& name) :
			m_stateName(name)
		{
		}

	public:
		virtual ~AState() = default;

		std::string const& getName()const
		{
			return m_stateName;
		}

		virtual void beginState() = 0;
		virtual void stepState() = 0;
		virtual void endState() = 0;
	private:
		std::string const m_stateName;
	};

	using StatePointer = std::unique_ptr<AState>;
	using StateCreator = std::function<StatePointer(A&& ...)>;

	void registerStateCreator(K const& key, StateCreator&& creator);

	template <class State>
	void registerStateType(K const& key);

	AState& currentState()const;
	K const& currentStateKey()const;
	bool hasState()const;

	void push(K const& key, A&& ... args);
	void change(K const& key, A&& ... args);
	void pop();
	void clear();
	/*! Should be called at each step. */
	void step();
private:
	using Event = std::function<void()>;

	struct StateHolder
	{
		StateHolder(StatePointer&& state, K const& key) :
			state(std::move(state)),
			key(key)
		{
		}

		StatePointer state;
		K const& key;
	};

	StatePointer createState(K const& key, A&& ... args);
	void processEvents();
	void pushEvent(Event&& event);

	void pushImp(K const& key, A&& ...args)
	{
		auto const creator = createState(key, std::forward<A>(args)...);
		auto state = createState(key, std::forward<A>(args)...);

		m_stack.emplace(std::move(state), key);
		m_stack.top().state->beginState();
	}

	void changeImp(K const& key, A&& ...args)
	{
		auto const creator = createState(key, std::forward<A>(args)...);
		auto state = createState(key, std::forward<A>(args)...);

		popImp();
		m_stack.emplace(std::move(state), key);
		m_stack.top().state->beginState();
	}

	void popImp()
	{
		if (!m_stack.empty())
		{
			m_stack.top().state->endState();
			m_stack.pop();
		}
	}

	void clearImp()
	{
		while (!m_stack.empty())
		{
			popImp();
		}
	}
private:
	std::map<K, StateCreator> m_creators;
	std::stack<StateHolder> m_stack;
	std::queue<Event> m_eventQueue;
};

#include "FiniteStateMachine.hxx"
#endif
