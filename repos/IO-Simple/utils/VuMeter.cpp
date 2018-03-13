#include "VuMeter.hpp"

#include <cmath>

VuMeter::VuMeter(rack::Vec const& pos, rack::Vec const& size) :
	m_currentValue(0.f)
{
	box.pos = pos;
	box.size = size;
}

void VuMeter::setValue(float cv)
{
	m_currentValue = std::abs(cv / 10.f);
	if (m_currentValue < std::numeric_limits<float>::epsilon())
		m_currentValue = 0.f;
	else if (m_currentValue > 1.f)
		m_currentValue = 1.f;
}

void VuMeter::draw(NVGcontext* vg)
{
	nvgSave(vg);
	nvgFillColor(vg, nvgRGBA(0x30, 0x33, 0x32, 0xFF));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y, 2.5f);
	nvgFill(vg);
	if (m_currentValue > 0.02f)
	{
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGBA(0x0, 0x80, 0x0, 0xFF));
		nvgRoundedRect(vg, 0.f, box.size.y - box.size.y * m_currentValue, box.size.x, box.size.y * m_currentValue, 2.5f);
		nvgFill(vg);
	}
	nvgRestore(vg);
}
