#include "ClockWidget.hpp"
#include "Clock.hpp"
#include "Simple.hpp"

#include <utils/TextDisplay.hpp>

ClockWidget::ClockWidget() :
	m_clock(new Clock),
	m_segmentDisplay(new TextDisplay)
{

	auto* const mainPanel = new rack::SVGPanel;
	auto const Margin = 5.f;

	box.size = rack::Vec(15 * 6, 380);
	m_segmentDisplay->box.pos = {25.f, 45.f};
	m_segmentDisplay->box.size = {15 * 6 - Margin - 25.f, 25.f};
	m_segmentDisplay->setFontSize(10.f);
	mainPanel->box.size = box.size;
    mainPanel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, "res/clock.svg")));
	setModule(m_clock);
	addChild(mainPanel);
	addChild(m_segmentDisplay);

	auto* const resetInput = createInput<rack::PJ301MPort>({}, Clock::INPUT_RESET);
	auto* const knob = createParam<rack::RoundBlackKnob>({}, Clock::PARAM_VALUE, 0.0001f, 1.f, 0.5f);
	createParam<rack::LEDButton>({5.f, 48.f}, Clock::PARAM_CHANGE_MODE, 0.f, 1.f, 0.f);

	resetInput->box.pos.x = 15.f * 6.f - resetInput->box.size.x - 10;
	resetInput->box.pos.y = 80.f;
	knob->box.pos.x = (15.f * 6.f - knob->box.size.x) / 2.f;
	knob->box.pos.y = resetInput->box.pos.y + resetInput->box.size.y + 5.f;

    float const initialYPos = knob->box.pos.y + knob->box.size.y + 15.f;
    float const outputMargin = 16.f;
    float const outputSpacing = 10.f;
	float yPos = initialYPos;

	for (auto i = 0u; i < 6u; ++i)
	{
		auto* const clockOutput = createOutput<rack::PJ301MPort>({}, Clock::OUTPUT_CLOCK_0 + i);
		clockOutput->box.pos.x = outputMargin;
		clockOutput->box.pos.y = yPos;
		yPos += clockOutput->box.size.y + outputSpacing;
	}
    yPos = initialYPos;
	for (auto i = 4u; i < 10u; ++i)
	{
		auto* const clockOutput = createOutput<rack::PJ301MPort>({}, Clock::OUTPUT_CLOCK_0 + i);
		clockOutput->box.pos.x = 15 * 6 - clockOutput->box.size.x - outputMargin;
		clockOutput->box.pos.y = yPos;
		yPos += clockOutput->box.size.y + outputSpacing;
	}
}

void ClockWidget::step()
{
	ExtendedModuleWidget::step();
	m_segmentDisplay->setText(m_clock->getCurrentText());
}
