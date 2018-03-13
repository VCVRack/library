#pragma once

#include "module-widget.h"

namespace DHE {

struct HostageWidget : public ModuleWidget {
  explicit HostageWidget(rack::Module *module);
};
}