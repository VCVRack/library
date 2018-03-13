#include "PluginInit.h"
#include "ButtonWithLight.h"
#include "dsp/digital.hpp"
#include "utils.h"
#include "SeqModule.h"

SEQWidget::SEQWidget()
{
    SEQ *module = new SEQ();
    setModule(module);
    box.size = Vec(15 * 22, 380);
    module->InitUI(this, box);
}

struct SEQGateModeItem : MenuItem
{
    SEQ *seq;
    SEQ::GateMode gateMode;
    void onAction()
    {
        seq->m_gateMode = gateMode;
    }
    void step() override
    {
        rightText = (seq->m_gateMode == gateMode) ? "✔" : "";
    }
};

struct SEQActionItem : MenuItem
{
    SEQ *seq;
    bool randomPitch = false;
    bool randomGate = false;
    bool randomSkip = false;
    void onAction()
    {
        seq->RandomizeHelper(randomPitch, randomGate, randomSkip);
    }
};

Menu *SEQWidget::createContextMenu()
{
    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    SEQ *seq = dynamic_cast<SEQ *>(module);
    assert(seq);

    SEQActionItem *triggerItem1 = new SEQActionItem();
    triggerItem1->text = "Randomize Pitch";
    triggerItem1->seq = seq;
    triggerItem1->randomPitch = true;
    menu->addChild(triggerItem1);

    SEQActionItem *triggerItem2 = new SEQActionItem();
    triggerItem2->text = "Randomize Gate";
    triggerItem2->seq = seq;
    triggerItem2->randomGate = true;
    menu->addChild(triggerItem2);

    SEQActionItem *triggerItem3 = new SEQActionItem();
    triggerItem3->text = "Randomize Skip";
    triggerItem3->seq = seq;
    triggerItem3->randomSkip = true;
    menu->addChild(triggerItem3);

    MenuLabel *modeLabel = new MenuLabel();
    modeLabel->text = "Gate Mode";
    menu->addChild(modeLabel);

    SEQGateModeItem *triggerItem = new SEQGateModeItem();
    triggerItem->text = "Trigger";
    triggerItem->seq = seq;
    triggerItem->gateMode = SEQ::TRIGGER;
    menu->addChild(triggerItem);

    SEQGateModeItem *retriggerItem = new SEQGateModeItem();
    retriggerItem->text = "Retrigger";
    retriggerItem->seq = seq;
    retriggerItem->gateMode = SEQ::RETRIGGER;
    menu->addChild(retriggerItem);

    SEQGateModeItem *continuousItem = new SEQGateModeItem();
    continuousItem->text = "Continuous";
    continuousItem->seq = seq;
    continuousItem->gateMode = SEQ::CONTINUOUS;
    menu->addChild(continuousItem);

    return menu;
}
