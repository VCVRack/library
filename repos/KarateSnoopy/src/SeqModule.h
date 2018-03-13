#pragma once
#include "PluginInit.h"
#include "SEQWidget.h"
#include "ButtonWithLight.h"
#include "dsp/digital.hpp"
#include "utils.h"
#include "LCDNumberWidget.h"
#include "TextLabelWidget.h"

#define MAX_STEPS 16

struct SEQ : Module
{
  public:
    enum ParamIds
    {
        PITCH_PARAM,                          // MAX_STEPS of these
        GATE_PARAM = PITCH_PARAM + MAX_STEPS, // MAX_STEPS of these
        SKIP_PARAM = GATE_PARAM + MAX_STEPS,  // MAX_STEPS of these
        EDIT_GATE_PARAM = SKIP_PARAM + MAX_STEPS,
        EDIT_PITCH_PARAM,
        EDIT_SKIP_PARAM,
        STEPS_PARAM,
        CLOCK_PARAM,
        RUN_PARAM,
        RESET_PARAM,
        PATTERN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        EXT_CLOCK_INPUT,
        RESET_INPUT,
        STEPS_INPUT,
        PATTERN_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CV_OUTPUT,
        GATE_X_OUTPUT,
        GATE_Y_OUTPUT,
        GATE_XORY_OUTPUT,
        NUM_OUTPUTS
    };

    enum GateMode
    {
        TRIGGER,
        RETRIGGER,
        CONTINUOUS,
    };

    enum LightIds
    {
        CV_LIGHT,
        GATE_LIGHT_0,                               // 16 of these
        LIGHT_IS_SKIP_0 = GATE_LIGHT_0 + 16,        // 16 of these
        LIGHT_IS_PITCH_ON_0 = LIGHT_IS_SKIP_0 + 16, // 16 of these
        GATE_X_LIGHT = LIGHT_IS_PITCH_ON_0 + 16,
        GATE_Y_LIGHT,
        GATE_X_OR_Y_LIGHT,
        LIGHT_RUNNING,
        LIGHT_RESET,
        LIGHT_EDIT_PITCH,
        LIGHT_EDIT_GATE,
        LIGHT_EDIT_SKIP,
        NUM_LIGHTS
    };

    ModuleWidget *m_moduleWidget = nullptr;
    bool m_running = true;
    bool m_initalized = false;
    ButtonWithLight m_pitchEditButton;
    ButtonWithLight m_gateEditButton;
    ButtonWithLight m_skipEditButton;
    ButtonWithLight m_runningButton;
    ButtonWithLight m_resetButton;
    SchmittTrigger m_clockTrigger; // for external clock
    PulseGenerator m_gatePulse;

    float m_phase = 0.0;
    int m_currentPattern = 0;
    int m_currentPatternIndex = 0;
    int m_currentStepIndex = 0;
    int m_lastStepIndex = 0;
    float m_isPitchOn[MAX_STEPS] = {0};
    float m_isSkip[MAX_STEPS] = {0};
    float m_stepLights[MAX_STEPS] = {0};
    float m_gateLights[MAX_STEPS] = {0};
    GateMode m_gateMode = TRIGGER;
    std::vector<Widget *> m_editPitchUI;
    std::vector<ParamWidget *> m_editPitchParamUI;
    std::vector<std::shared_ptr<ButtonWithLight>> m_editGateUI;
    std::vector<std::shared_ptr<ButtonWithLight>> m_editSkipUI;
    std::vector<std::vector<int>> m_patterns;

    float m_cvLight = 0.0f;
    float m_gateXLight = 0.0f;
    float m_gateYLight = 0.0f;
    float m_gateXorYLight = 0.0f;

    SEQ();
    void step() override;
    bool ProcessClockAndReset();
    void ShowEditPitchUI(bool showUI);
    void ShowEditGateUI(bool showUI);
    void ShowEditSkipUI(bool showUI);
    void ProcessUIButtons();
    void AdvanceStep();
    void ProcessXYTriggers();
    void ProcessEditButtons();
    void FadeGateLights();
    void RandomizeHelper(bool randomPitch, bool randomGate, bool randomSkip);
    void UpdateLights();

    void InitUI(ModuleWidget *moduleWidget, Rect box);
    Widget *addChild(Widget *widget);
    ParamWidget *addParam(ParamWidget *param);
    Port *addInput(Port *input);
    Port *addOutput(Port *output);

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
    void reset() override;
    void randomize() override;
};
