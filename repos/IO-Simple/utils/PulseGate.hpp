#if!defined PULSEGATE_HPP
#define PULSEGATE_HPP

class PulseGate
{
public:
	explicit PulseGate(float pulseDuration = 250.f);

	void reset();
	bool process(bool gate);
private:
	/*! The pulse duration in second. */
	float m_pulseDuration;
	float m_current = 0.f;
	bool m_pulse = false;
};

class Pulser
{
public:
	explicit Pulser(float pulseDuration = 0.001f) :
		m_gate(pulseDuration)
	{
	}

	void trigger()
	{
		m_trigger = true;
	}

	void reset()
	{
		m_gate.reset();
	}

	bool step()
	{
		auto const result = m_gate.process(m_trigger);

		m_trigger = false;
		return result;
	}
private:
	PulseGate m_gate;
	bool m_trigger = false;
};

#endif
