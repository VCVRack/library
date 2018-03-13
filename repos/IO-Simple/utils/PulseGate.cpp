#include "PulseGate.hpp"
#include <engine.hpp>
#include <iostream>
#include <limits>
#include <cmath>

PulseGate::PulseGate(float pulseDuration) :
	m_pulseDuration(pulseDuration)
{
}

void PulseGate::reset()
{
	m_current = 0.f;
	m_pulse = false;
}

bool PulseGate::process(bool gate)
{
	m_current += 1.f / static_cast<float>(rack::engineGetSampleRate());
	if (gate)
	{
		m_current = 0.f;
		m_pulse = true;
	}
	if (m_current > m_pulseDuration || std::abs(m_current - m_pulseDuration) < std::numeric_limits<float>::epsilon())
	{
		m_pulse = false;
	}
	return m_pulse;
}
