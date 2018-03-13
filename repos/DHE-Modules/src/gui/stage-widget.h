#pragma once

#include "module-widget.h"

namespace DHE {

struct StageWidget : public ModuleWidget {
  explicit StageWidget(rack::Module *module);
};
}