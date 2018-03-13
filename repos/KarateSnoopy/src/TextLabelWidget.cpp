#include "TextLabelWidget.h"

#define FONT_FILE assetPlugin(plugin, "res/Roboto-Black.ttf")

TextLabelWidget::TextLabelWidget(
    int x,
    int y,
    int dx,
    int dy,
    int fontSize,
    int fontSpacing,
    NVGcolor fontColor,
    bool renderBackground,
    std::string value)
{
    m_font = Font::load(FONT_FILE);
    box.pos = Vec(x, y);
    box.size = Vec(dx, dy);

    m_value = value;
    m_renderBackground = renderBackground;
    m_fontSize = fontSize;
    m_fontSpacing = fontSpacing;
    m_fontColor = fontColor;
}

void TextLabelWidget::draw(NVGcontext *vg)
{
    if (m_renderBackground)
    {
        NVGcolor backgroundColor = nvgRGB(0x00, 0x00, 0x44); // dark blue background
        NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);
        nvgStrokeWidth(vg, 1.0);
        nvgStrokeColor(vg, borderColor);
        nvgStroke(vg);
    }

    nvgFontSize(vg, m_fontSize);
    nvgFontFaceId(vg, m_font->handle);
    nvgTextLetterSpacing(vg, m_fontSpacing);

    Vec textPos = Vec(0.0f, 0.0f);
    if (m_renderBackground)
    {
        textPos = Vec(7.0f, 25.0f);
    }
    nvgFillColor(vg, m_fontColor);
    nvgText(vg, textPos.x, textPos.y, m_value.c_str(), NULL);
}
