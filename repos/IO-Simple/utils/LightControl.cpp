#include "LightControl.hpp"
#include <engine.hpp>

LightControl::LightControl()
{
	setState<StateOff>();
}

void LightControl::step()
{
	m_currentValue = m_currentState->step();
}

float LightControl::lightValue()const
{
	return m_currentValue;
}

float LightControl::StateOff::step()
{
	return 0.f;
}

float LightControl::StateOn::step()
{
	return 1.f;
}

LightControl::StateBlink::StateBlink(float blinkTime, bool initialLightState) :
	m_blinkTime(blinkTime),
	m_timeCounter(0.f),
	m_lightState(initialLightState)
{
}

float LightControl::StateBlink::step()
{
	auto const timeStep = 1.f / rack::engineGetSampleRate();

	m_timeCounter += timeStep;
	if (m_timeCounter > m_blinkTime)
	{
		m_timeCounter -= m_blinkTime;
		m_lightState = !m_lightState;
	}
	return m_lightState ? 1.0f : 0.f;
}
