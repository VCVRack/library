#include "Simple.hpp"
#include <dsp/digital.hpp>

struct ButtonTrigger : rack::Module
{
	enum ParamIds
	{
		TRIGGER,
		NUM_PARAMS
	};

	enum OutputIds
	{
		OUTPUT_0,
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		NUM_OUTPUTS
	};

	ButtonTrigger() : Module(NUM_PARAMS, 0u, NUM_OUTPUTS){}

	void step() override;

private:
	rack::SchmittTrigger m_buttonTrigger;
};

void ButtonTrigger::step()
{
	auto const buttonValue = params[TRIGGER].value;
	auto const schmittValue = m_buttonTrigger.process(buttonValue);

	for (auto& output : outputs)
	{
		if (output.active)
		{
			output.value = schmittValue;
		}
	}
}

namespace Helpers
{
	template <class T>
	static T* centerHorizontaly(T* const widget, float const panelWidth)
	{
		auto const widgetWidth = widget->box.size.x;
		auto const position = (panelWidth - widgetWidth) / 2.f;

		widget->box.pos.x = position;
		return widget;
	}
}

ButtonTriggerWidget::ButtonTriggerWidget()
{
	static constexpr float const Margin = 5.f;
	static constexpr float const Top = 50.f;
	static constexpr float const ButtonSize = 28.f;

	ButtonTrigger* const module = new ButtonTrigger;

	box.size = rack::Vec(15 * 4, 380);

	setModule(module);

	auto* const mainPanel = new rack::SVGPanel();
	mainPanel->box.size = box.size;
	mainPanel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, "res/button_trigger.svg")));
	addChild(mainPanel);

	addChild(rack::createScrew<rack::ScrewSilver>({0, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 15, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({0, box.size.y - 15}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 15, box.size.y - 15}));

	addParam(Helpers::centerHorizontaly(rack::createParam<rack::CKD6>(rack::Vec(Margin, Top), module, ButtonTrigger::TRIGGER, 0.f, 1.f, 0.f), box.size.x));

	rack::Vec pos(0, Top + 40.f);

	for (auto i = 0u; i < ButtonTrigger::NUM_OUTPUTS; ++i)
	{
		addOutput(Helpers::centerHorizontaly(rack::createOutput<rack::PJ3410Port>(pos, module, i), box.size.x));
		pos.y += ButtonSize + Margin;
	}
}
