#pragma once

#include "module-widget.h"

namespace DHE {

struct SwaveWidget : public ModuleWidget {
  explicit SwaveWidget(rack::Module *module);
};
}