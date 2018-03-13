#pragma once

#include "rack.hpp"
#include "asset.hpp"
#include "componentlibrary.hpp"

extern Plugin *plugin;

struct Jack : SVGPort {
  Jack() {
    background->svg = SVG::load(assetPlugin(plugin, "res/components/Jack.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct Davies1900hRedSnapKnob : Davies1900hRedKnob {
  Davies1900hRedSnapKnob ( ) {
    snap = true;
  }
};
