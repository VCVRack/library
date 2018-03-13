#include "Simple.hpp"
#include <utils/PulseGate.hpp>
#include <utils/Algorithm.hpp>
#include <utils/SimpleHelpers.hpp>
#include <dsp/digital.hpp>

#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>

struct ClockDivider;

namespace
{
	static constexpr int const MinDivider = 1u;
	static constexpr int const MaxDivider = 128u;

	class ClockDividerImp
	{
	public:
		explicit ClockDividerImp(unsigned int index, unsigned int limit = 1u) :
			m_lightPulse(0.2f),
			m_index(index),
			m_limit(limit)
		{
		}

		void setLimit(unsigned int limit){ m_limit = limit;	}

		void reset()
		{
			m_current = 0u;
			m_lightPulse.reset();
		}

		void process(bool const clockTrigger, ClockDivider& module);
	private:
		float getModulationValue(rack::Param const& param, float modulation)const
		{
			return param.value * std::floor(modulation) / 10.f;
		}
	private:
		PulseGate m_lightPulse;
		unsigned int const m_index;
		unsigned int m_current = 0u;
		unsigned int m_limit = 1u;
		float m_modulation = 1.f;
	};
}

struct ClockDivider : rack::Module
{
	enum ParamsIds
	{
		CLOCK_DIVIDER_0,
		CLOCK_DIVIDER_1,
		CLOCK_DIVIDER_2,
		CLOCK_DIVIDER_3,
		CLOCK_DIVIDER_4,
		CLOCK_DIVIDER_5,
		CLOCK_DIVIDER_6,
		CLOCK_DIVIDER_7,
		CLOCK_MOD_DIVIDER_0,
		CLOCK_MOD_DIVIDER_1,
		CLOCK_MOD_DIVIDER_2,
		CLOCK_MOD_DIVIDER_3,
		CLOCK_MOD_DIVIDER_4,
		CLOCK_MOD_DIVIDER_5,
		CLOCK_MOD_DIVIDER_6,
		CLOCK_MOD_DIVIDER_7,
		NUM_PARAMS
	};

	enum InputIds
	{
		INPUT_CLOCK,
		INPUT_RESET,
		INPUT_CLOCK_MOD_DIVIDER_0,
		INPUT_CLOCK_MOD_DIVIDER_1,
		INPUT_CLOCK_MOD_DIVIDER_2,
		INPUT_CLOCK_MOD_DIVIDER_3,
		INPUT_CLOCK_MOD_DIVIDER_4,
		INPUT_CLOCK_MOD_DIVIDER_5,
		INPUT_CLOCK_MOD_DIVIDER_6,
		INPUT_CLOCK_MOD_DIVIDER_7,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUTPUT_CLOCK_0,
		OUTPUT_CLOCK_1,
		OUTPUT_CLOCK_2,
		OUTPUT_CLOCK_3,
		OUTPUT_CLOCK_4,
		OUTPUT_CLOCK_5,
		OUTPUT_CLOCK_6,
		OUTPUT_CLOCK_7,
		OUTPUT_RESET,
		OUTPUT_CLOCK,
		NUM_OUTPUTS
	};

	static constexpr unsigned int const ClockDividerCount = 8u;

	ClockDivider() :
		Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 8u)
	{
		auto initClockDivider = 1u;

		for (auto i = 0u; i < ClockDividerCount; ++i)
		{
			m_clockDividers.emplace_back(i, std::min(128u, initClockDivider));
			initClockDivider *= 2u;
		};
	}

	void step() override
	{
		auto const& inputClock = inputs.at(INPUT_CLOCK);
		auto const& inputReset = inputs.at(INPUT_RESET);
		bool const clockTick = inputClock.active && m_clockTrigger.process(inputClock.value);

		if (inputReset.active && m_resetTrigger.process(inputReset.value))
		{
			reset();
		}
		for (auto& divider : m_clockDividers)
		{
			divider.process(clockTick, *this);
		}

		outputs.at(OUTPUT_CLOCK).value = inputClock.value;
		outputs.at(OUTPUT_RESET).value = inputReset.value;
	}

	void reset() override final
	{
		std::for_each(m_clockDividers.begin(), m_clockDividers.end(),
					  [](ClockDividerImp& imp) { imp.reset(); });
	}

	void randomize() override final
	{
		reset();
	}
private:
	std::vector<ClockDividerImp> m_clockDividers;
	rack::SchmittTrigger m_resetTrigger;
	rack::SchmittTrigger m_clockTrigger;
};

struct ClockDividerKnob : rack::RoundSmallBlackSnapKnob
{
	void connectLabel(rack::Label* label)
	{
		linkedLabel = label;
		if (linkedLabel)
		{
			linkedLabel->text = formatCurrentValue();
		}
	}

	void onChange(rack::EventChange& e) override
	{
		rack::RoundSmallBlackSnapKnob::onChange(e);
		if (linkedLabel)
		{
			linkedLabel->text = formatCurrentValue();
		}
	}
private:
	std::string formatCurrentValue()const
	{
		return "/" + std::to_string(static_cast<unsigned int>(value));
	}
private:
	rack::Label* linkedLabel = nullptr;
};

void ClockDividerImp::process(bool const clockTrigger, ClockDivider& module)
{
	auto const& diviserParam = module.params.at(ClockDivider::CLOCK_DIVIDER_0 + m_index);
	auto const& diviserModParam = module.params.at(ClockDivider::CLOCK_MOD_DIVIDER_0 + m_index);
	auto const& diviserMod = module.inputs.at(ClockDivider::INPUT_CLOCK_MOD_DIVIDER_0 + m_index);
	auto& output = module.outputs.at(m_index);
	auto& light = module.lights.at(m_index);
	auto gate = false;
	auto const value = clamp<float>(diviserParam.value + getModulationValue(diviserModParam, getInputValue(diviserMod)), MinDivider, MaxDivider);

	m_limit = static_cast<unsigned int>(value);
	if (clockTrigger)
	{
		++m_current;
		if (m_current >= m_limit)
		{
			gate = true;
			m_current = 0u;
		}
	}
	output.value = gate ? 10.f : 0.f;
	light.value = m_lightPulse.process(gate) ? 1.0 : 0.0;
}

namespace Helpers
{
	template <class InputPortClass>
	static rack::Port* addInput(rack::ModuleWidget* const widget, rack::Module* module, int const inputId,
								rack::Vec const& position, std::string const& label, float labelOffset,
								float* light)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);

		if (!label.empty())
		{
			rack::Label* const labelWidget = new rack::Label;

			labelWidget->box.pos = position;
			labelWidget->box.pos.x += labelOffset;
			labelWidget->text = label;
			widget->addChild(labelWidget);
		}

		port->box.pos = position;

		widget->addInput(port);
		return port;
	}
}

ClockDividerWidget::ClockDividerWidget()
{
	static constexpr float const Margin = 5.f;

	auto* const module = new ClockDivider;

	box.size = rack::Vec(15 * 10, 380);

	setModule(module);

	auto* const mainPanel = new rack::SVGPanel;

	mainPanel->box.size = box.size;
	mainPanel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, "res/clock_divider.svg")));
	addChild(mainPanel);

	addChild(rack::createScrew<rack::ScrewSilver>({15, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({15, box.size.y - 15}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, box.size.y - 15}));

	// Setup input ports
	auto* inputReset = createInput<rack::PJ301MPort>({0, 45}, ClockDivider::INPUT_RESET);
	auto* outputReset = createOutput<rack::PJ301MPort>({0, 90}, ClockDivider::OUTPUT_RESET);

	auto* inputClock = createInput<rack::PJ301MPort>({0, 45}, ClockDivider::INPUT_CLOCK);
	auto* outputClock = createOutput<rack::PJ301MPort>({0, 90}, ClockDivider::OUTPUT_CLOCK);

	auto const horMargin = (mainPanel->box.size.x - (inputReset->box.size.x + inputClock->box.size.x + 15)) / 2.f;
	inputReset->box.pos.x = horMargin;
	outputReset->box.pos.x = horMargin;
	inputClock->box.pos.x = mainPanel->box.size.x - inputClock->box.size.x - horMargin;
	outputClock->box.pos.x = mainPanel->box.size.x - outputClock->box.size.x - horMargin;

	auto defaultDividerValue = 1u;
	auto const left = 10;
	rack::Vec pos{left, 125};

	// Setup clock outputs port and controls
	for (auto i = 0u; i < 8u; ++i)
	{
		auto* clockControl = createParam<ClockDividerKnob>(pos, ClockDivider::CLOCK_DIVIDER_0 + i, 1.f, MaxDivider, defaultDividerValue);

		clockControl->snap = true;
		pos.x += clockControl->box.size.x + Margin;

		auto modControlPos = pos;

		modControlPos.y = pos.y + clockControl->box.size.y / 4.f;

		auto* clockModControl = createParam<rack::RoundSmallBlackKnob>(modControlPos, ClockDivider::CLOCK_MOD_DIVIDER_0 + i, -MaxDivider, MaxDivider, 0.f);

		clockModControl->snap = true;
		pos.x += clockModControl->box.size.x / 2.f + Margin;

		auto* const inputPortWidget = createInput<rack::PJ301MPort>(pos, ClockDivider::INPUT_CLOCK_MOD_DIVIDER_0 + i);

		pos.x += inputPortWidget->box.size.x + Margin;

		auto* const outputPortWidget = createOutput<rack::PJ301MPort>(pos, i);

		rack::Vec lightPos = pos;

		lightPos.x += 20.f;
		lightPos.y += 2.f;

		createLight<rack::SmallLight<rack::RedLight>>(lightPos, i);

		pos.x += outputPortWidget->box.size.x;

		auto* const textWidget = new rack::Label;

		clockControl->box.size = outputPortWidget->box.size;
		clockModControl->box.size = outputPortWidget->box.size.mult(0.5f);
		clockControl->connectLabel(textWidget);
		textWidget->box.pos = pos;
		textWidget->box.pos.x += -3;
		textWidget->box.pos.y += 2;

		addChild(textWidget);

		pos.x = left;
		pos.y += 30;
		defaultDividerValue *= 2u;
	}
	initialize();
}
