#include "LCDNumberWidget.h"

#define FONT_FILE assetPlugin(plugin, "res/Segment7Standard.ttf")

LCDNumberWidget::LCDNumberWidget(int x, int y, int *pvalue)
{
    font = Font::load(FONT_FILE);
    box.pos = Vec(x, y);
    value = pvalue;
    box.size = Vec(52, 32);
}

void LCDNumberWidget::draw(NVGcontext *vg)
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

    nvgFontSize(vg, 20);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::string to_display = std::to_string(*value);
    Vec textPos = Vec(7.0f, 25.0f);

    // Skipping text background to fake real LCD display

    // NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    // nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    // nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    // textColor = nvgRGB(0xda, 0xe9, 0x29);
    // nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    // nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);

    NVGcolor textColor = nvgRGB(0xFF, 0xFF, 0xFF); // white text
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
}
