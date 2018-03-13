#pragma once

#include "rack.hpp"
using namespace rack;
#include "PluginInit.h"

struct TextLabelWidget : TransparentWidget
{
    std::string m_value;
    std::shared_ptr<Font> m_font;
    int m_fontSize;
    float m_fontSpacing;
    NVGcolor m_fontColor;
    bool m_renderBackground;

    TextLabelWidget(
        int x, int y, int dx, int dy,
        int fontSize,
        int fontSpacing,
        NVGcolor fontColor,
        bool renderBackground,
        std::string value);
    void draw(NVGcontext *vg) override;
};
