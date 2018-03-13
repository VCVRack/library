#include <cassert>

#include <iostream>

template <class K, class ... A>
void FiniteStateMachine<K, A...>::registerStateCreator(K const& key, StateCreator&& creator)
{
	m_creators.emplace(key, std::move(creator));
	std::cout << "Registered state: " << std::to_string(key) << std::endl;
}

template <class K, class ... A>
template <class State>
void FiniteStateMachine<K, A...>::registerStateType(K const& key)
{
	auto creator = [](A&& ... args)
		{
			return std::unique_ptr<AState>{new State{std::forward<A>(args)...}};
		};
	registerStateCreator(key, std::move(creator));
}

template <class K, class ... A>
auto FiniteStateMachine<K, A...>::currentState()const -> AState&
{
	assert( !m_stack.empty() );

	return *(m_stack.top().state);
}

template <class K, class ... A>
K const& FiniteStateMachine<K, A...>::currentStateKey()const
{
	assert( !m_stack.empty() );

	return m_stack.top().key;
}

template <class K, class ... A>
bool FiniteStateMachine<K, A...>::hasState()const
{
	return !m_stack.empty();
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::push(K const& key, A&& ... args)
{
	auto arguments = std::make_tuple(std::forward<A>(args)...);

	pushEvent([&, key]()
	{
		pushImp(key, std::forward<A>(args)...);
	});
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::change(K const& key, A&& ... args)
{
	pushEvent([&, key]()
	{
		changeImp(key, std::forward<A>(args)...);
	});
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::pop()
{
	pushEvent([this]()
	{
		popImp();
	});
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::clear()
{
	pushEvent([this]()
	{
		clearImp();
	});
}

template <class K, class ... A>
auto FiniteStateMachine<K, A...>::createState(K const& key, A&& ... args) -> StatePointer
{
	auto creatorIt = m_creators.find(key);

	assert( creatorIt != m_creators.end() );

	return creatorIt->second(std::forward<A>(args)...);
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::processEvents()
{
	while (!m_eventQueue.empty())
	{
		m_eventQueue.front()();
		m_eventQueue.pop();
	}
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::pushEvent(Event&& event)
{
	m_eventQueue.emplace(std::move(event));
}

template <class K, class ... A>
void FiniteStateMachine<K, A...>::step()
{
	processEvents();
}
