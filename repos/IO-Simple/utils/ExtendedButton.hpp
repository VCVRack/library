#if!defined EXTENDEDBUTTON_HPP
#define EXTENDEDBUTTON_HPP
#include <type_traits>
#include <dsp/digital.hpp>
#include <functional>

template <class T>
class ExtendedButton : public T
{
	static_assert( std::is_base_of<rack::Widget, T>::value, "T must be a rack::Widget" );
public:
	using Callback = std::function<void()>;

	ExtendedButton()
	{
	}

	explicit ExtendedButton(Callback&& callback) :
		m_callback(std::move(callback))
	{
	}

	void setCallback(Callback&& callback)
	{
		m_callback = std::move(callback);
	}

	void onChange(rack::EventChange& e) override
	{
		T::onChange(e);
		if (m_trigger.process(T::value) && m_callback)
		{
			m_callback();
		}
	}
private:
	rack::SchmittTrigger m_trigger;
	Callback m_callback;
};

#endif
