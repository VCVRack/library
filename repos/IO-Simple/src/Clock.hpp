#if!defined CLOCK_HPP
#define CLOCK_HPP
#include <rack.hpp>
#include <dsp/digital.hpp>
#include <utils/FiniteStateMachine.hpp>
#include <chrono>
#include <array>

class Clock : public rack::Module
{
	static std::vector<std::pair<unsigned int, std::string>> const Resolutions;
	static unsigned int const Resolution;
	static unsigned int const MaxClockPosition;

	class ClockState;
	class ChangeBPMState;
	class ChangeRatioState;
	class ClockOutput;

	template <unsigned int Index> class ChangeGateTimeState;
	template <unsigned int Index> class ChangeDivisorState;
	template <unsigned int Index> class ChangeOutputVoltageState;
	template <unsigned int Index> class ChangeResolutionState;
public:
	static std::chrono::nanoseconds const OneSecond;
	static constexpr std::size_t ClockOutputCount = 12u;

	enum InputIds
	{
		INPUT_RESET,
		NUM_INPUTS
	};

	enum ParamIds
	{
		PARAM_CHANGE_MODE,
		PARAM_VALUE,
		NUM_PARAMS
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
		OUTPUT_CLOCK_8,
		OUTPUT_CLOCK_9,
		OUTPUT_CLOCK_10,
		OUTPUT_CLOCK_11,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS
	};

	enum StateIds : unsigned int
	{
		STATE_BPM,

		STATE_RESOLUTION_0,
		STATE_DIVISION_0,
		STATE_GATE_TIME_0,
		STATE_VOLTAGE_0,

		STATE_RESOLUTION_1,
		STATE_DIVISION_1,
		STATE_GATE_TIME_1,
		STATE_VOLTAGE_1,

		STATE_RESOLUTION_2,
		STATE_DIVISION_2,
		STATE_GATE_TIME_2,
		STATE_VOLTAGE_2,

		STATE_RESOLUTION_3,
		STATE_DIVISION_3,
		STATE_GATE_TIME_3,
		STATE_VOLTAGE_3,

		STATE_RESOLUTION_4,
		STATE_DIVISION_4,
		STATE_GATE_TIME_4,
		STATE_VOLTAGE_4,

		STATE_RESOLUTION_5,
		STATE_DIVISION_5,
		STATE_GATE_TIME_5,
		STATE_VOLTAGE_5,

		STATE_RESOLUTION_6,
		STATE_DIVISION_6,
		STATE_GATE_TIME_6,
		STATE_VOLTAGE_6,

		STATE_RESOLUTION_7,
		STATE_DIVISION_7,
		STATE_GATE_TIME_7,
		STATE_VOLTAGE_7,

		STATE_RESOLUTION_8,
		STATE_DIVISION_8,
		STATE_GATE_TIME_8,
		STATE_VOLTAGE_8,

		STATE_RESOLUTION_9,
		STATE_DIVISION_9,
		STATE_GATE_TIME_9,
		STATE_VOLTAGE_9,

		STATE_RESOLUTION_10,
		STATE_DIVISION_10,
		STATE_GATE_TIME_10,
		STATE_VOLTAGE_10,

		STATE_RESOLUTION_11,
		STATE_DIVISION_11,
		STATE_GATE_TIME_11,
		STATE_VOLTAGE_11,

		STATE_COUNT
	};

	using FSM = FiniteStateMachine<unsigned int, Clock&>;

	Clock();

	void restart();
	void reset() override;
	void step() override;

	std::chrono::nanoseconds getInterval()const;
	std::string getCurrentText()const;

	void setGateTime(unsigned int const divisorIndex, std::chrono::nanoseconds const time);
	std::chrono::nanoseconds getGateTime(unsigned int const divisorIndex)const;

	void setDivisor(unsigned int const index, unsigned int const divisor);
	unsigned int getDivisor(unsigned int const index)const;

	void setOutputVoltage(unsigned int const index, float const voltage);
	float getOutputVoltage(unsigned int const index) const;

	void setResolutionIndex(unsigned int const index, std::size_t const resolutionIndex);
	std::size_t getResolutionIndex(unsigned int const index) const;
private:
	void updateClockTrigger();
	void updateCurrentState();

	json_t *toJson() override;
	void fromJson(json_t *root) override;

	ClockOutput& getOutput(unsigned int const index);
	ClockOutput const& getOutput(unsigned int const index)const;
private:
	class ClockOutput
	{
	public:
		bool step(int const clockPosition, std::chrono::nanoseconds const dt);

		void restart();
		void recallDefaultValues();

		void setDivisor(unsigned int divisor);
		unsigned int getDivisor()const;

		void setGateTime(std::chrono::nanoseconds const& time);
		std::chrono::nanoseconds getGateTime()const;

		void setOutputVoltage(float const voltage);
		float getOutputVoltage()const;

		void setResolutionIndex(std::size_t const index);
		std::size_t getResolutionIndex()const;

		json_t *toJson()const;
		void fromJson(json_t *root);
	private:
		bool gateStep(std::chrono::nanoseconds const dt);
	private:
		std::chrono::nanoseconds m_gateTime;
		std::chrono::nanoseconds m_currentGateTime;
		float m_outputVoltage = 10.f;
		unsigned int m_divisor = 1u;
		std::size_t m_resolutionIndex = 3u;
	};
private:
	FSM m_machine;
	std::chrono::nanoseconds m_interval;
	std::chrono::nanoseconds m_current;
	std::chrono::nanoseconds m_increment;
	std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> m_lastTime;
	int m_clockPosition = 0;
	float m_previousValue = 0.f;
	rack::SchmittTrigger m_inputResetTrigger;
	rack::SchmittTrigger m_buttonTrigger;
	std::array<ClockOutput, ClockOutputCount> m_outputs;
	bool m_clockTrigger = false;
};

//////////////////////////////////////////////////////////
//
// class Clock::ClockState
//
class Clock::ClockState : public Clock::FSM::AState
{
public:
	explicit ClockState(std::string&& stateName, Clock& clock) :
		AState(std::move(stateName)),
		m_clock(clock)
	{
	}

	void beginState() override
	{
		m_currentText = formatCurrentText();
	}
	void stepState() override {}
	void endState() override {}
	void setValue(float const value)
	{
		onValueChanged(value);
		m_currentText = formatCurrentText();
	}

	std::string const& getCurrentText()const
	{
		return m_currentText;
	}
private:
	virtual std::string formatCurrentText()const = 0;
	virtual void onValueChanged(float const value) = 0;
protected:
	template <class ... T>
	static std::string formatValue(char const* const format, T&& ... values)
	{
		static std::unique_ptr<char[]> Buffer{new char[64]};

		std::snprintf(Buffer.get(), 64, format, std::forward<T>(values)...);
		return Buffer.get();
	}

	Clock& clock() { return m_clock; }
	Clock const& clock() const { return m_clock; }

	void setInterval(std::chrono::nanoseconds interval)
	{
		m_clock.m_interval = interval;
	}

	std::chrono::nanoseconds getInterval()const
	{
		return m_clock.getInterval();
	}

	void setGateTime(unsigned int const index, std::chrono::nanoseconds const time)
	{
		m_clock.setGateTime(index, time);
	}

	std::chrono::nanoseconds getGateTime(unsigned int const index)const
	{
		return m_clock.getGateTime(index);
	}

	void setDivisor(unsigned int const index, unsigned int const divisor)
	{
		m_clock.setDivisor(index, divisor);
	}

	unsigned int getDivisor(unsigned int const index)const
	{
		return m_clock.getDivisor(index);
	}

	void setOutputVoltage(unsigned int const index, float const voltage)
	{
		m_clock.setOutputVoltage(index, voltage);
	}

	float getOutputVoltage(unsigned int const index)const
	{
		return m_clock.getOutputVoltage(index);
	}

	void reset()
	{
		m_clock.restart();
	}
private:
	Clock& m_clock;
	std::string m_currentText;
};

#endif
