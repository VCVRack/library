#pragma once
#include "rack.hpp"
using namespace rack;

struct SEQWidget : ModuleWidget
{
    SEQWidget();
    Menu *createContextMenu() override;
};
