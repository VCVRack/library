#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct WidgetAutomaton : ModuleWidget {
    WidgetAutomaton();
    Menu *createContextMenu() override;
};

struct WidgetChaos : ModuleWidget {
    WidgetChaos();
    Menu *createContextMenu() override;
};

struct WidgetByte : ModuleWidget {
    WidgetByte();
};

struct WidgetScaler : ModuleWidget {
    WidgetScaler();
};

struct WidgetXFade : ModuleWidget {
    WidgetXFade();
};

struct WidgetOr : ModuleWidget {
    WidgetOr();
};

struct WidgetNot : ModuleWidget {
    WidgetNot();
};

struct WidgetXor : ModuleWidget {
    WidgetXor();
};

struct WidgetNews : ModuleWidget {
    WidgetNews();
};
struct WidgetMix : ModuleWidget {
    WidgetMix();
};

struct WidgetColumn : ModuleWidget {
    WidgetColumn();
    Menu *createContextMenu() override;
};

struct WidgetGate : ModuleWidget {
    WidgetGate();
};

struct WidgetWrap : ModuleWidget {
    WidgetWrap();
};
