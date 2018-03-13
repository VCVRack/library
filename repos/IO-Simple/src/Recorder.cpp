#include "rack.hpp"
#include "Simple.hpp"

#include <utils/WavWriter.hpp>
#include <utils/SimpleHelpers.hpp>
#include <utils/ExtendedButton.hpp>
#include <utils/LightControl.hpp>
#include <utils/StateMachine.hpp>
#include <utils/Memory.hpp>
#include <utils/Path.hpp>
#include <utils/VuMeter.hpp>

#include <dsp/digital.hpp>
#include <../ext/osdialog/osdialog.h>

#include <iostream> // DEBUG
#include <cstdlib>
#include <array>
#include <cmath>

class Recorder : public rack::Module
{
public:
	static char const* const NoneLabel;

	enum InputIds
	{
		INPUT_LEFT_IN = 0,
		INPUT_RIGHT_IN,
		INPUT_START_STOP,
		NUM_INPUTS
	};

	enum ParamIds
	{
		PARAM_RECORD_ARM = 0,
		PARAM_START_STOP,
		PARAM_INPUT_VOLUME,
		PARAM_SELECT_FILE,
		NUM_PARAMS
	};

	enum OutputIds
	{
		OUTPUT_START_STOP = 0,
		OUTPUT_RECORD_ARM,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		MAIN_LIGHT = 0,
		FILE_LIGHT,
		NUM_LIGHTS
	};

	enum StateIds
	{
		INITIAL_STATE = 0u,
		ARMED_STATE,
		RECORD_STATE
	};

	Recorder() :
		rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		m_stateMachine.addState(INITIAL_STATE, [this](StateMachine& machine)
				{
					auto const armValue = params[PARAM_RECORD_ARM].value;

					if (hasOutputFilePath() && m_armTrigger.process(armValue))
					{
						machine.change(ARMED_STATE);
					}
				});
		m_stateMachine.addStateBegin(INITIAL_STATE, [this]()
				{
					m_redLightControl.setState<LightControl::StateOff>();
				});
		m_stateMachine.addState(ARMED_STATE, [this](StateMachine& machine)
				{
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const armValue = params[PARAM_RECORD_ARM].value;
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);

					if (m_armTrigger.process(armValue))
					{
						machine.change(INITIAL_STATE);
					}
					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(RECORD_STATE);
						outputs[OUTPUT_START_STOP].value = 1.f;
					}
				});
		m_stateMachine.addStateBegin(ARMED_STATE, [this]()
				{
					m_redLightControl.setState<LightControl::StateBlink>(0.5f, false);
				});
		m_stateMachine.addState(RECORD_STATE, [this](StateMachine& machine)
				{
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);
					auto const& leftInput = inputs[INPUT_LEFT_IN];
					auto const& rightInput = inputs[INPUT_RIGHT_IN];

					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(INITIAL_STATE);
						outputs[OUTPUT_START_STOP].value = 1.f;
					}

					WavWriter::Frame frame;

					frame.samples[0u] = getInputValue(leftInput) / 10.f;
					frame.samples[1u] = getInputValue(rightInput) / 10.f;
					m_writer.push(frame);
					if (m_writer.haveError())
					{
						// TODO: error notification
						std::cerr << "Recorder error: " << WavWriter::getErrorText(m_writer.error()) << std::endl;
						m_writer.clearError();
						machine.change(INITIAL_STATE);
					}
				});
		m_stateMachine.addStateBegin(RECORD_STATE, [this]()
				{
					startRecording();
					m_redLightControl.setState<LightControl::StateOn>();
				});
		m_stateMachine.addStateEnd(RECORD_STATE, [this]()
				{
					stopRecording();
				});
		m_stateMachine.change(INITIAL_STATE);
	}

	float leftInputValue()const
	{
		return inputs.at(INPUT_LEFT_IN).active ? inputs.at(INPUT_LEFT_IN).value : 0.f;
	}

	float rightInputValue()const
	{
		return inputs.at(INPUT_RIGHT_IN).active ? inputs.at(INPUT_RIGHT_IN).value : 0.f;
	}

	void setOutputFilePath(std::string const& path)
	{
		m_outputFilePath = path;
	}

	bool hasOutputFilePath()const
	{
		return !m_outputFilePath.empty();
	}

	bool isArmed()const
	{
		return m_stateMachine.currentIndex() == ARMED_STATE;
	}

	bool isRecording()const
	{
		return m_writer.isRunning();
	}

	void startRecording()
	{
		m_writer.start(m_outputFilePath);
	}

	void stopRecording()
	{
		m_writer.stop();
	}

	void onSampleRateChange() override
	{
		m_writer.stop();
		// TODO: error notification
		std::cerr << "Recorder error: the sample rate has changed during the recording" << std::endl;
	}

	void step() override
	{
		outputs.at(OUTPUT_START_STOP).value = 0.f;
		m_stateMachine.step();
		m_redLightControl.step();
		lights.at(FILE_LIGHT).value = (m_outputFilePath.empty() || m_outputFilePath == Recorder::NoneLabel) ? 0.f : 1.f;
		lights.at(MAIN_LIGHT).value = m_redLightControl.lightValue();
	}
private:
	WavWriter m_writer;
	StateMachine m_stateMachine;
	LightControl m_redLightControl;
	rack::SchmittTrigger m_startStopTrigger;
	rack::SchmittTrigger m_armTrigger;
	std::string m_outputFilePath;
};

char const* const Recorder::NoneLabel = "<none>";

namespace Helpers
{
	template <class InputPortClass>
	static rack::Port* addAudioInput(rack::ModuleWidget* const widget, rack::Module* const module,
								 	 int const inputId, rack::Vec const& position,
								 	 std::string const& label)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);
		auto* const labelWidget = new rack::Label;

		labelWidget->text = label;
		widget->addInput(port);
		widget->addChild(labelWidget);

		float const portSize = port->box.size.x;

		labelWidget->box.pos.x = position.x;
		labelWidget->box.pos.y = position.y + portSize;
		return port;
	}
}



RecorderWidget::RecorderWidget() :
	m_recorder(new Recorder),
	m_label(new rack::Label),
	m_leftMeter(new VuMeter({20.f, 180.f}, {15.f, 130.f})),
	m_rightMeter(new VuMeter({15.f * 6.f - 15.f - 20.f, 180.f}, {15.f, 130.f}))
{
	static constexpr float const PortSize = 24.6146f;
	static constexpr float const Spacing = 10.f;
	static constexpr float const Width = 15.f * 6.f;

	auto* const mainPanel = new rack::SVGPanel;

	box.size = rack::Vec(15 * 6, 380);
	mainPanel->box.size = box.size;
	mainPanel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, "res/recorder.svg")));
	addChild(mainPanel);

	addChild(rack::createScrew<rack::ScrewSilver>({15, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({15, box.size.y - 15}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, box.size.y - 15}));
	addChild(m_leftMeter);
	addChild(m_rightMeter);

	setModule(m_recorder);
	{
		static constexpr float const Left = (Width - (PortSize * 2.f + Spacing)) / 2.f;

		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_LEFT_IN, {Left, 315}, "L");
		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_RIGHT_IN, {Left + Spacing + PortSize, 315}, "R");
	}

	static constexpr float const Top = 90;

	auto* const selectFileButton = createParam<ExtendedButton<rack::LEDButton>>({10, Top - 30}, Recorder::PARAM_SELECT_FILE, 0.f, 1.f, 0.f);

	createParam<rack::LEDButton>({10, Top}, Recorder::PARAM_RECORD_ARM, 0.f, 1.f, 0.f);
	createParam<rack::LEDButton>({40, Top}, Recorder::PARAM_START_STOP, 0.f, 1.f, 0.f);
	createInput<rack::PJ301MPort>({37, Top + 25}, Recorder::INPUT_START_STOP);
	createOutput<rack::PJ301MPort>({37, Top + 55}, Recorder::OUTPUT_START_STOP);
	createLight<rack::SmallLight<rack::RedLight>>(rack::Vec{68, Top + 6}, Recorder::MAIN_LIGHT);
	createLight<rack::TinyLight<rack::GreenLight>>(rack::Vec{16.5f, Top - 23.5f}, Recorder::FILE_LIGHT);

	m_label->text = Recorder::NoneLabel;
	m_label->box.pos.x = 22;
	m_label->box.pos.y = Top - 32;
	selectFileButton->setCallback(std::bind(&RecorderWidget::onSelectFileButtonClicked, this));

	addChild(m_label);
}

void RecorderWidget::onSelectFileButtonClicked()
{
    std::cout << "armed: " << m_recorder->isArmed() << std::endl;
    std::cout << "recording: " << m_recorder->isRecording() << std::endl;
	if (!m_recorder->isArmed() && !m_recorder->isRecording())
	{
		selectOutputFile();
	}
}

bool RecorderWidget::selectOutputFile()
{
	std::unique_ptr<char[], FreeDeleter<char>> path{osdialog_file(OSDIALOG_SAVE, ".", "output.wav", nullptr)};
	bool result = false;

	if (path)
	{
		std::string pathStr{path.get()};

		if (Path::extractExtension(pathStr).empty())
		{
			pathStr.append(".wav");
		}
		setOutputFilePath(pathStr);
		result = true;
	}
	else
	{
		setOutputFilePath(Recorder::NoneLabel);
	}
	return result;
}

void RecorderWidget::setOutputFilePath(std::string const& outputFilePath)
{
	m_recorder->setOutputFilePath(outputFilePath);
	m_label->text = Path::extractFileName(outputFilePath);
}

void RecorderWidget::step()
{
	m_leftMeter->setValue(m_recorder->leftInputValue());
	m_rightMeter->setValue(m_recorder->rightInputValue());
	ExtendedModuleWidget::step();
}
