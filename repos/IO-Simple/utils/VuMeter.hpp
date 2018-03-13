#if!defined VUMETER_HPP
#define VUMETER_HPP
#include <rack.hpp>

class VuMeter : public rack::TransparentWidget
{
public:
	VuMeter(rack::Vec const& pos, rack::Vec const& size);

	void setValue(float cv);
	void draw(NVGcontext* vg) override;
private:
	float m_currentValue;
};

#endif
