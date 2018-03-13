#pragma once

#include "asset.hpp"
#include "rack.hpp"

extern Plugin *plugin;

struct SKRoundWhiteKnob : RoundKnob {
  SKRoundWhiteKnob() {
    setSVG(SVG::load(assetPlugin(plugin, "res/components/RoundWhite.svg")));
    box.size = Vec(38, 38);
  }
};

struct SKRoundSmallWhiteKnob : SKRoundWhiteKnob {
  SKRoundSmallWhiteKnob() {
    box.size = Vec(28, 28);
  }
};

struct SKRoundLargeWhiteKnob : SKRoundWhiteKnob {
  SKRoundLargeWhiteKnob() {
    box.size = Vec(46, 46);
  }
};

struct SKRoundHugeWhiteKnob : SKRoundWhiteKnob {
  SKRoundHugeWhiteKnob() {
    box.size = Vec(56, 56);
  }
};

struct SKRoundSmallWhiteSnapKnob : SKRoundSmallWhiteKnob {
  SKRoundSmallWhiteSnapKnob() {
    snap = true;
  }
};

struct SKRoundTinyWhiteKnob : SKRoundWhiteKnob {
  SKRoundTinyWhiteKnob() {
    box.size = Vec(18, 18);
  }
};

struct SKRoundTinyWhiteSnapKnob : SKRoundTinyWhiteKnob {
  SKRoundTinyWhiteSnapKnob() {
    snap = true;
  }
};
