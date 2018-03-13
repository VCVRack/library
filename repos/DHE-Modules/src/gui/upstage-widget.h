#pragma once

#include "module-widget.h"

namespace DHE {

struct UpstageWidget : public ModuleWidget {
  explicit UpstageWidget(rack::Module *module);
};
}