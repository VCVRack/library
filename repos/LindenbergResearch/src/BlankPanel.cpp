#include "LindenbergResearch.hpp"
#include "Release.h"


struct BlankPanel : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    LCDWidget *lcd1 = new LCDWidget(LCD_COLOR_FG, 15);


    BlankPanel() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}


    void step() override;
};


void BlankPanel::step() {
}


/**
 * @brief Blank Panel with Logo
 */
struct BlankPanelWidget : LRModuleWidget {
    BlankPanelWidget(BlankPanel *module);
};


BlankPanelWidget::BlankPanelWidget(BlankPanel *module) : LRModuleWidget(module) {
    //setPanel(SVG::load(assetPlugin(plugin, "res/BlankPanel.svg")));

    panel = new LRPanel(10,10);
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/BlankPanel.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** LCD *************
   /* module->lcd1->box.pos = Vec(34, 355);
    addChild(module->lcd1);
    module->lcd1->text = VERSION_STR;*/
    // ***** LCD *************
}


Model *modelBlankPanel = Model::create<BlankPanel, BlankPanelWidget>("Lindenberg Research", "BlankPanel 01", "Blank Panel 20TE", BLANK_TAG);
