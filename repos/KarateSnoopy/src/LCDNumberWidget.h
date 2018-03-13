#pragma once

#include "rack.hpp"
using namespace rack;
#include "PluginInit.h"

struct LCDNumberWidget : TransparentWidget
{
    int *value;
    std::shared_ptr<Font> font;

    LCDNumberWidget(int x, int y, int *pvalue);
    void draw(NVGcontext *vg) override;
};
