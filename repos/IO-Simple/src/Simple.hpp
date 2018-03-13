#if!defined SIMPLE_HPP
#define SIMPLE_HPP
#include <rack.hpp>
#include <widgets.hpp>

#include <utils/ExtendedModuleWidget.hpp>
#include <utils/VuMeter.hpp>

extern rack::Plugin *plugin;

class ButtonTriggerWidget : public rack::ModuleWidget
{
public:
	ButtonTriggerWidget();
};

class ClockDividerWidget : public ExtendedModuleWidget
{
public:
	ClockDividerWidget();
};

class Recorder;

class RecorderWidget : public ExtendedModuleWidget
{
public:
	RecorderWidget();

	void step() override;
private:
	void onSelectFileButtonClicked();
	bool selectOutputFile();
	void setOutputFilePath(std::string const& outputFilePath);
private:
	Recorder* const m_recorder;
	rack::Label* const m_label;
	VuMeter* const m_leftMeter;
	VuMeter* const m_rightMeter;
};

#endif
