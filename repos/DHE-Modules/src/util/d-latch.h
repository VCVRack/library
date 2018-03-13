#pragma once

#include <functional>
#include <vector>

#include "latch.h"

namespace DHE {

/**
 * A latch that can be set and reset.
 */
struct DLatch : Latch {
  /**
   * Sets the latch HIGH. Fires 'rise' if the latch was not already HIGH.
   */
  void set() { set_state(State::HIGH); }

  /**
   * Sets the latch LOW. Fires 'fall' if the latch was not already LOW.
   */
  void reset() { set_state(State::LOW); }
};
}