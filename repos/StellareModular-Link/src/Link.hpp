/*

 VCV-Link by Stellare Modular
 Copyright (C) 2017-2018 - Vincenzo Pietropaolo, Sander Baan
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/

#include "rack.hpp"

using namespace rack;

extern Plugin* plugin;

struct BlueSmallButton : SVGSwitch, MomentarySwitch
{
    BlueSmallButton()
    {
        addFrame(SVG::load(assetPlugin(plugin, "res/BlueSmallButton_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/BlueSmallButton_1.svg")));

        sw->wrap();
        box.size = sw->box.size;
    }
};

struct KnobSimpleWhite : SVGKnob
{
    KnobSimpleWhite()
    {
        box.size = Vec(28, 28);
        minAngle = -0.82 * M_PI;
        maxAngle = 0.82 * M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/KnobSimpleWhite.svg")));
    }
};

struct LinkWidget : ModuleWidget
{
    LinkWidget();
};
