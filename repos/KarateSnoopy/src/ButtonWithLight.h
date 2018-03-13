#pragma once

#include "rack.hpp"
using namespace rack;
#include "dsp/digital.hpp"

class ButtonWithLight
{
  public:
    void Init(ModuleWidget *moduleWidget, Module *module, int x, int y, int paramId, float *pValue, bool bigButton, int lightId);
    void SetOnOff(bool onOff, bool currentState);
    void AddInput(int inputId);
    float GetState();
    bool Process(std::vector<Param> &params);
    bool ProcessWithInput(std::vector<Param> &params, std::vector<Input> &input);
    bool ProcessHelper(float value);
    void SetVisible(bool showUI);

    bool m_log = false;

  private:
    std::vector<Widget *> m_controls;
    float m_light = 0.0;
    int m_lightId = 0;
    Module *m_module = nullptr;
    float *m_pValue = nullptr;
    int m_paramId = 0;
    int m_inputId = -1;
    SchmittTrigger m_trigger;
    bool m_onOffType = false;
};
