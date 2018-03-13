#if!defined CLOCKWIDGET_HPP
#define CLOCKWIDGET_HPP
#include <utils/ExtendedModuleWidget.hpp>

class Clock;
class TextDisplay;

class ClockWidget : public ExtendedModuleWidget
{
public:
	ClockWidget();

	void step() override;
private:
	Clock* const m_clock;
	TextDisplay* const m_segmentDisplay;
};

#endif
