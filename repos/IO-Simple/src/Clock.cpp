#include "Clock.hpp"
#include <utils/Conversions.hpp>
#include <algorithm>

std::chrono::nanoseconds const Clock::OneSecond{1000000000};

namespace
{
	json_t* serialize(std::chrono::nanoseconds const value)
	{
		auto const text = std::to_string(value.count());

		return json_string(text.c_str());
	}

	bool deserialize(json_t* const json, std::chrono::nanoseconds& value)
	{
		bool result = false;

		if (json_is_string(json))
		{
			char const* const text = json_string_value(json);

			value = std::chrono::nanoseconds{std::stoull(text)};
			result = true;
		}
		return result;
	}
}

//////////////////////////////////////////////////////////
//
// class Clock
//

unsigned int const Clock::Resolution = 128u;
unsigned int const Clock::MaxClockPosition = Clock::Resolution * 512u * 16u;

std::vector<std::pair<unsigned int, std::string>> const Clock::Resolutions =
{
	{Clock::Resolution * 16, "4 / 1"},
	{Clock::Resolution * 8, "2 / 1"},
	{Clock::Resolution * 4, "1 / 1"},
	{Clock::Resolution * 2, "1 / 2"},
	{Clock::Resolution, "1 / 4"},
	{Clock::Resolution / 3, "1 / 4T"},
	{Clock::Resolution / 2, "1 / 8"},
	{Clock::Resolution / 6, "1 / 8T"},
	{Clock::Resolution / 4, "1 / 16"},
	{Clock::Resolution / 12, "1 / 16T"},
	{Clock::Resolution / 8, "1 / 32"},
	{Clock::Resolution / 24, "1 / 32T"},
	{Clock::Resolution / 16, "1 / 64"},
	{Clock::Resolution / 48, "1 / 64T"},
	{Clock::Resolution / 32, "1 / 128"},
	{Clock::Resolution / 96, "1 / 128T"}
};

Clock::Clock() :
	rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
	m_interval(OneSecond)
{
	m_machine.registerStateType<ChangeBPMState>(Clock::STATE_BPM);

	m_machine.registerStateType<ChangeResolutionState<0u>>(Clock::STATE_RESOLUTION_0);
	m_machine.registerStateType<ChangeDivisorState<0u>>(Clock::STATE_DIVISION_0);
	m_machine.registerStateType<ChangeGateTimeState<0u>>(Clock::STATE_GATE_TIME_0);
	m_machine.registerStateType<ChangeOutputVoltageState<0u>>(Clock::STATE_VOLTAGE_0);

	m_machine.registerStateType<ChangeResolutionState<1u>>(Clock::STATE_RESOLUTION_1);
	m_machine.registerStateType<ChangeDivisorState<1u>>(Clock::STATE_DIVISION_1);
	m_machine.registerStateType<ChangeGateTimeState<1u>>(Clock::STATE_GATE_TIME_1);
	m_machine.registerStateType<ChangeOutputVoltageState<1u>>(Clock::STATE_VOLTAGE_1);

	m_machine.registerStateType<ChangeResolutionState<2u>>(Clock::STATE_RESOLUTION_2);
	m_machine.registerStateType<ChangeDivisorState<2u>>(Clock::STATE_DIVISION_2);
	m_machine.registerStateType<ChangeGateTimeState<2u>>(Clock::STATE_GATE_TIME_2);
	m_machine.registerStateType<ChangeOutputVoltageState<2u>>(Clock::STATE_VOLTAGE_2);

	m_machine.registerStateType<ChangeResolutionState<3u>>(Clock::STATE_RESOLUTION_3);
	m_machine.registerStateType<ChangeDivisorState<3u>>(Clock::STATE_DIVISION_3);
	m_machine.registerStateType<ChangeGateTimeState<3u>>(Clock::STATE_GATE_TIME_3);
	m_machine.registerStateType<ChangeOutputVoltageState<3u>>(Clock::STATE_VOLTAGE_3);

	m_machine.registerStateType<ChangeResolutionState<3u>>(Clock::STATE_RESOLUTION_4);
	m_machine.registerStateType<ChangeDivisorState<3u>>(Clock::STATE_DIVISION_4);
	m_machine.registerStateType<ChangeGateTimeState<3u>>(Clock::STATE_GATE_TIME_4);
	m_machine.registerStateType<ChangeOutputVoltageState<3u>>(Clock::STATE_VOLTAGE_4);

	m_machine.registerStateType<ChangeResolutionState<5u>>(Clock::STATE_RESOLUTION_5);
	m_machine.registerStateType<ChangeDivisorState<5u>>(Clock::STATE_DIVISION_5);
	m_machine.registerStateType<ChangeGateTimeState<5u>>(Clock::STATE_GATE_TIME_5);
	m_machine.registerStateType<ChangeOutputVoltageState<5u>>(Clock::STATE_VOLTAGE_5);

	m_machine.registerStateType<ChangeResolutionState<6u>>(Clock::STATE_RESOLUTION_6);
	m_machine.registerStateType<ChangeDivisorState<6u>>(Clock::STATE_DIVISION_6);
	m_machine.registerStateType<ChangeGateTimeState<6u>>(Clock::STATE_GATE_TIME_6);
	m_machine.registerStateType<ChangeOutputVoltageState<6u>>(Clock::STATE_VOLTAGE_6);

	m_machine.registerStateType<ChangeResolutionState<7u>>(Clock::STATE_RESOLUTION_7);
	m_machine.registerStateType<ChangeDivisorState<7u>>(Clock::STATE_DIVISION_7);
	m_machine.registerStateType<ChangeGateTimeState<7u>>(Clock::STATE_GATE_TIME_7);
	m_machine.registerStateType<ChangeOutputVoltageState<7u>>(Clock::STATE_VOLTAGE_7);

	m_machine.registerStateType<ChangeResolutionState<8u>>(Clock::STATE_RESOLUTION_8);
	m_machine.registerStateType<ChangeDivisorState<8u>>(Clock::STATE_DIVISION_8);
	m_machine.registerStateType<ChangeGateTimeState<8u>>(Clock::STATE_GATE_TIME_8);
	m_machine.registerStateType<ChangeOutputVoltageState<8u>>(Clock::STATE_VOLTAGE_8);

	m_machine.registerStateType<ChangeResolutionState<9u>>(Clock::STATE_RESOLUTION_9);
	m_machine.registerStateType<ChangeDivisorState<9u>>(Clock::STATE_DIVISION_9);
	m_machine.registerStateType<ChangeGateTimeState<9u>>(Clock::STATE_GATE_TIME_9);
	m_machine.registerStateType<ChangeOutputVoltageState<9u>>(Clock::STATE_VOLTAGE_9);

	m_machine.registerStateType<ChangeResolutionState<10u>>(Clock::STATE_RESOLUTION_10);
	m_machine.registerStateType<ChangeDivisorState<10u>>(Clock::STATE_DIVISION_10);
	m_machine.registerStateType<ChangeGateTimeState<10u>>(Clock::STATE_GATE_TIME_10);
	m_machine.registerStateType<ChangeOutputVoltageState<10u>>(Clock::STATE_VOLTAGE_10);

	m_machine.registerStateType<ChangeResolutionState<11u>>(Clock::STATE_RESOLUTION_11);
	m_machine.registerStateType<ChangeDivisorState<11u>>(Clock::STATE_DIVISION_11);
	m_machine.registerStateType<ChangeGateTimeState<11u>>(Clock::STATE_GATE_TIME_11);
	m_machine.registerStateType<ChangeOutputVoltageState<11u>>(Clock::STATE_VOLTAGE_11);

	m_machine.change(Clock::STATE_BPM, *this);
    m_previousValue = params.at(Clock::PARAM_VALUE).value;
	restart();
}

void Clock::restart()
{
	m_current = std::chrono::nanoseconds{0u};
	m_lastTime = std::chrono::steady_clock::now();
	m_previousValue = params[PARAM_VALUE].value;
	m_clockPosition = 0u;
	std::for_each(m_outputs.begin(), m_outputs.end(), [](ClockOutput& output){ output.restart(); });
}

void Clock::reset()
{
	m_interval = std::chrono::nanoseconds{OneSecond / 2};
	m_machine.change(Clock::STATE_BPM, *this);
	std::for_each(m_outputs.begin(), m_outputs.end(), [](ClockOutput& output){ output.recallDefaultValues(); });
	restart();
}

void Clock::updateClockTrigger()
{
	auto const currentTime = std::chrono::steady_clock::now();
	auto const elaspedTime = currentTime - m_lastTime;
	auto const interval = getInterval();

	m_lastTime = currentTime;
	m_current += elaspedTime;
	if (m_current >= interval)
	{
		m_current = std::chrono::nanoseconds{0};
		// The maximum divisor is 512 so we can have at most
		// Resolution * 512u ticks to count.
		m_clockPosition = (m_clockPosition + 1) % (MaxClockPosition);
		m_clockTrigger = true;
	}
	else if (m_inputResetTrigger.process(inputs.at(INPUT_RESET).value))
	{
		restart();
		m_clockTrigger = true;
	}
	else
	{
		m_clockTrigger = false;
	}
	if (m_clockTrigger)
	{
		for (auto i = 0u; i < m_outputs.size(); ++i)
		{
			auto& output = getOutput(i);
			auto const gate = output.step(m_clockPosition, elaspedTime);

			outputs.at(OUTPUT_CLOCK_0 + i).value = gate ? output.getOutputVoltage() : 0.f;
		}
	}
}

void Clock::updateCurrentState()
{
	if (m_buttonTrigger.process(params.at(Clock::PARAM_CHANGE_MODE).value))
	{
		unsigned int const newStateKey = (m_machine.currentStateKey() + 1u) % STATE_COUNT;

		m_machine.change(newStateKey, *this);
	}
}

void Clock::step()
{
	rack::Module::step();
	m_machine.step();

	updateClockTrigger();
	updateCurrentState();

	if (m_machine.hasState())
	{
		auto& currentState = static_cast<Clock::ClockState&>(m_machine.currentState());
		auto const currentValue = params.at(Clock::PARAM_VALUE).value;
        auto const delta = currentValue - m_previousValue;

		if (!(std::abs(delta) < std::numeric_limits<float>::epsilon()))
		{
			currentState.setValue(currentValue);
		}
		m_previousValue = currentValue;
	}
}

std::chrono::nanoseconds Clock::getInterval()const
{
	return m_interval / Resolution;
}

std::string Clock::getCurrentText()const
{
	std::string text;

	if (m_machine.hasState())
	{
		auto& currentState = static_cast<Clock::ClockState&>(m_machine.currentState());

		text = currentState.getCurrentText();
	}
	return text;
}

void Clock::setGateTime(unsigned int const divisorIndex, std::chrono::nanoseconds const time)
{
	getOutput(divisorIndex).setGateTime(time);
}

std::chrono::nanoseconds Clock::getGateTime(unsigned int const divisorIndex)const 
{
	return getOutput(divisorIndex).getGateTime();
}

json_t *Clock::toJson()
{
	json_t* const rootNode = json_object();
	json_t* const outputArrayNode = json_array();

	json_object_set_new(rootNode, "interval", serialize(m_interval));
	json_object_set_new(rootNode, "state", json_integer(m_machine.currentStateKey()));
	json_object_set_new(rootNode, "outputs", outputArrayNode);

	for (auto const& output : m_outputs)
	{
		json_array_append(outputArrayNode, output.toJson());
	}
	return rootNode;
}

void Clock::fromJson(json_t *root)
{
	json_t* const intervalNode = json_object_get(root, "interval");
	json_t* const stateNode = json_object_get(root, "state");
	json_t* const outputArrayNode = json_object_get(root, "outputs");

	if (intervalNode && json_is_string(intervalNode) &&
		stateNode && json_is_integer(stateNode) &&
		outputArrayNode && json_is_array(outputArrayNode))
	{
		deserialize(intervalNode, m_interval);
		m_machine.change(json_integer_value(stateNode), *this);

		std::size_t index = 0u;
		json_t* value = nullptr;

		json_array_foreach(outputArrayNode, index, value)
		{
			m_outputs[index].fromJson(value);
		}
	}
}

auto Clock::getOutput(unsigned int const index) -> ClockOutput&
{
	assert( index < m_outputs.size() );

	return m_outputs[index];
}

auto Clock::getOutput(unsigned int const index)const -> ClockOutput const&
{
	assert( index < m_outputs.size() );

	return m_outputs[index];
}

void Clock::setDivisor(unsigned int const index, unsigned int const divisor)
{
	getOutput(index).setDivisor(divisor);
}

unsigned int Clock::getDivisor(unsigned int const index)const
{
	return getOutput(index).getDivisor();
}

void Clock::setOutputVoltage(unsigned int const index, float const voltage)
{
	getOutput(index).setOutputVoltage(voltage);
}

float Clock::getOutputVoltage(unsigned int const index) const
{
	return getOutput(index).getOutputVoltage();
}

void Clock::setResolutionIndex(unsigned int const index, std::size_t const resolutionIndex)
{
	getOutput(index).setResolutionIndex(resolutionIndex);
}

std::size_t Clock::getResolutionIndex(unsigned int const index) const
{
	return getOutput(index).getResolutionIndex();
}

//////////////////////////////////////////////////////////
//
// class Clock::Output
//
bool Clock::ClockOutput::step(int const clockPosition, std::chrono::nanoseconds const dt)
{
	auto const resolution = Resolutions[m_resolutionIndex].first;

	if (clockPosition % (m_divisor * resolution) == 0)
	{
		m_currentGateTime = std::chrono::nanoseconds{0u};
	}
	return gateStep(dt);
}

void Clock::ClockOutput::restart()
{
	m_currentGateTime = std::chrono::nanoseconds{0u};
}

void Clock::ClockOutput::recallDefaultValues()
{
	m_gateTime = std::chrono::nanoseconds{0};
	m_outputVoltage = 10.f;
	m_divisor = 1u;
	m_resolutionIndex = 4u;
}

void Clock::ClockOutput::setDivisor(unsigned int divisor)
{
	assert (divisor > 0);

	m_divisor = divisor;
}

unsigned int Clock::ClockOutput::getDivisor()const
{
	return m_divisor;
}

void Clock::ClockOutput::setGateTime(std::chrono::nanoseconds const& time)
{
	m_gateTime = time;
}

std::chrono::nanoseconds Clock::ClockOutput::getGateTime()const
{
	return m_gateTime;
}

void Clock::ClockOutput::setOutputVoltage(float const voltage)
{
	m_outputVoltage = voltage;
}

float Clock::ClockOutput::getOutputVoltage()const
{
	return m_outputVoltage;
}

void Clock::ClockOutput::setResolutionIndex(std::size_t const index)
{
	assert( index < Resolutions.size() );
	m_resolutionIndex = index;
}

std::size_t Clock::ClockOutput::getResolutionIndex()const
{
	return m_resolutionIndex;
}

bool Clock::ClockOutput::gateStep(std::chrono::nanoseconds const dt)
{
	bool result = false;

	if (m_currentGateTime <= m_gateTime)
	{
		m_currentGateTime += dt;
		result = true;
	}
	return result;
}

json_t *Clock::ClockOutput::toJson()const
{
	json_t *rootNode = json_object();

	json_object_set_new(rootNode, "gate_time", serialize(m_gateTime));
	json_object_set_new(rootNode, "output_voltage", json_real(m_outputVoltage));
	json_object_set_new(rootNode, "divisor", json_integer(m_divisor));
	json_object_set_new(rootNode, "resolution_index", json_integer(m_resolutionIndex));
	return rootNode;
}

void Clock::ClockOutput::fromJson(json_t *root)
{
	auto* const gateTimeNode = json_object_get(root, "gate_time");
	auto* const outputVoltageNode = json_object_get(root, "output_voltage");
	auto* const divisorNode = json_object_get(root, "divisor");
	auto* const resolutionIndexNode = json_object_get(root, "resolution_index");

	if (gateTimeNode && outputVoltageNode && divisorNode && resolutionIndexNode &&
		json_is_string(gateTimeNode) && json_is_real(outputVoltageNode) &&
		json_is_integer(divisorNode) && json_is_integer(resolutionIndexNode))
	{
		deserialize(gateTimeNode, m_gateTime);
		m_outputVoltage = json_real_value(outputVoltageNode);
		m_divisor = json_integer_value(divisorNode);
		m_resolutionIndex = json_integer_value(resolutionIndexNode);
	}
}

//////////////////////////////////////////////////////////
//
// class Clock::ChangeBPMState
//
class Clock::ChangeBPMState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
	static constexpr char const* const Format = "Main\n  %u BPM";
public:
	explicit ChangeBPMState(Clock& clock) :
		ClockState("BPM", clock)
	{
	}
private:
	std::string formatCurrentText() const override
	{
		auto const bpm = nanosecondToBpm(clock().getInterval() * Clock::Resolution);

		return formatValue(Format, static_cast<unsigned int>(bpm));
	}

	void onValueChanged(float const value) override
	{
		auto const bpm = m_minBPM + (m_maxBPM - m_minBPM) * value;

		setInterval(bpmToNanoseconds(bpm));
	}
private:
	unsigned int const m_minBPM = 1u;
	unsigned int const m_maxBPM = 300u;
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeDivisorState
//
template <unsigned int Index>
class Clock::ChangeDivisorState : public ClockState
{
	static constexpr char const* const Format = "Divisor %u\n/%u";
public:
	explicit ChangeDivisorState(Clock& clock) :
		ClockState("Divisor", clock)
	{
	}
private:
	std::string formatCurrentText() const override
	{
		return formatValue(Format, Index, clock().getDivisor(Index));
	}

	void onValueChanged(float const value) override
	{
		auto const divisor = m_minDivisor + static_cast<unsigned int>(static_cast<float>(m_maxDivisor - m_minDivisor) * value);

		setDivisor(Index, divisor);
	}
private:
	unsigned int m_minDivisor = 1u;
	unsigned int m_maxDivisor = 512u;
};


//////////////////////////////////////////////////////////
//
// class Clock::ChangeResolutionState
//
template <unsigned int Index>
class Clock::ChangeResolutionState : public ClockState
{
	static constexpr char const* const Format = "Resolution %u\n  %s";
public:
	explicit ChangeResolutionState(Clock& clock) :
		ClockState("Resolution", clock)
	{
	}
private:
	std::string formatCurrentText() const override
	{
		auto const resolutionIndex = clock().getResolutionIndex(Index);

		return formatValue(Format, Index, Resolutions[resolutionIndex].second.c_str());
	}

	void onValueChanged(float const value) override
	{
		assert(Resolutions.size() > 0);

		std::size_t resolutionIndex = static_cast<float>(Clock::Resolutions.size() - 1) * value;

		clock().setResolutionIndex(Index, resolutionIndex);
	}
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeGateTimeState
//
template <unsigned int Index>
class Clock::ChangeGateTimeState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
	static constexpr char const* const Format = "Gate time %u\n  %.3fs";
public:
	explicit ChangeGateTimeState(Clock& clock) :
		ClockState("Gate time", clock)
	{
	}
private:
	std::string formatCurrentText() const override
	{
		auto const gateTime = std::chrono::duration_cast<Seconds>(getGateTime(Index));

		return formatValue(Format, Index, gateTime.count());
	}

	void onValueChanged(float const value) override
	{
		auto const gateTime = m_minTime + (m_maxTime - m_minTime) * value;

		setGateTime(Index, std::chrono::duration_cast<std::chrono::nanoseconds>(gateTime));
	}
private:
	Seconds const m_minTime{0.f};
	Seconds const m_maxTime{10.f};
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeOutputVoltageState
//
template <unsigned int Index>
class Clock::ChangeOutputVoltageState : public ClockState
{
	static constexpr char const* const Format = "Voltage %u\n  %.2fv";
public:
	explicit ChangeOutputVoltageState(Clock& clock) :
		ClockState("Output voltage", clock)
	{
	}

private:
	std::string formatCurrentText() const override
	{
		auto const voltage = getOutputVoltage(Index);

		return formatValue(Format, Index, voltage);
	}

	void onValueChanged(float const value) override
	{
		auto const voltage = m_min + (m_max - m_min) * value;

		setOutputVoltage(Index, voltage);
	}
private:
	float const m_min = 0.f;
	float const m_max = 10.f;
};
