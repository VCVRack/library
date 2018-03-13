#if!defined LIGHTCONTROL_HPP
#define LIGHTCONTROL_HPP
#include <memory>

class LightControl
{
public:
	class AState
	{
	public:
		virtual ~AState() = default;

		virtual float step() = 0;
	};

	class StateOff;
	class StateOn;
	class StateBlink;

	LightControl();

	void step();
	float lightValue()const;

	template <class T, class ... A>
	void setState(A&& ... args)
	{
		m_currentState.reset(new T(std::forward<A>(args)...));
	}
private:
	std::unique_ptr<AState> m_currentState;
	float m_currentValue = 0.f;
};

class LightControl::StateOff : public LightControl::AState
{
public:
	float step() override;
};

class LightControl::StateOn : public LightControl::AState
{
public:
	float step() override;
};

class LightControl::StateBlink : public LightControl::AState
{
public:
	explicit StateBlink(float blinkTime, bool initialLightState = true);

	float step() override;
private:
	float const m_blinkTime;
	float m_timeCounter;
	bool m_lightState;
};

#endif
