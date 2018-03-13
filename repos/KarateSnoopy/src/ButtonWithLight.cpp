#include "ButtonWithLight.h"
#include "utils.h"

void ButtonWithLight::Init(ModuleWidget *moduleWidget, Module *module, int x, int y, int paramId, float *pValue, bool bigButton, int lightId)
{
    ParamWidget *p;
    m_lightId = lightId;
    m_module = module;
    if (bigButton)
    {
        p = createParam<PB61303>(Vec(x, y), module, paramId, 0.0, 1.0, 0.0);
    }
    else
    {
        p = createParam<LEDButton>(Vec(x, y), module, paramId, 0.0, 1.0, 0.0);
    }
    moduleWidget->addParam(p);
    m_controls.push_back(p);

    m_pValue = pValue;
    if (m_pValue == nullptr)
        m_pValue = &m_light;
    int delta = (bigButton) ? 11 : 6;
    auto p2 = createLight<SmallLight<GreenLight>>(Vec(x + delta, y + delta), module, lightId);
    moduleWidget->addChild(p2);
    module->lights[lightId].value = *m_pValue;
    m_controls.push_back(p2);

    m_paramId = paramId;
}

void ButtonWithLight::SetOnOff(bool onOff, bool currentState)
{
    m_onOffType = onOff;
    *m_pValue = currentState ? 1.0 : 0.0;
}

void ButtonWithLight::SetVisible(bool showUI)
{
    for (auto &c : m_controls)
    {
        c->visible = showUI;
    }
}

void ButtonWithLight::AddInput(int inputId)
{
    m_inputId = inputId;
}

float ButtonWithLight::GetState()
{
    return *m_pValue;
}

bool ButtonWithLight::Process(std::vector<Param> &params)
{
    return ProcessHelper(params[m_paramId].value);
}

bool ButtonWithLight::ProcessWithInput(std::vector<Param> &params, std::vector<Input> &input)
{
    return ProcessHelper(params[m_paramId].value + input[m_inputId].value);
}

bool ButtonWithLight::ProcessHelper(float value)
{
    bool returnValue = false;
    if (m_trigger.process(value))
    {
        returnValue = true;
    }

    const float lightLambda = 0.075;

    if (m_onOffType)
    {
        if (returnValue)
        {
            if (*m_pValue > 0.0f)
                *m_pValue = 0.0f;
            else
                *m_pValue = 1.0f;
        }

        if (m_log)
        {
            write_log(0, "returnValue: %d m_pValue:%f\n", returnValue, *m_pValue);
        }
    }
    else
    {
        if (returnValue)
        {
            *m_pValue = 1.0f;
        }
        *m_pValue -= *m_pValue / lightLambda / engineGetSampleRate();

        if (m_log)
        {
            write_log(0, "returnValue: %d m_pValue:%f\n", returnValue, *m_pValue);
        }
    }

    m_module->lights[m_lightId].value = *m_pValue;

    return returnValue;
}
