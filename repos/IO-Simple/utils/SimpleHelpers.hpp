#if!defined SIMPLEHELPERS_HPP
#define SIMPLEHELPERS_HPP
#include <rack.hpp>
#include <type_traits>

static inline float getInputValue(rack::Input const& input, float const inactiveValue = 0.f)
{
	return input.active ? input.value : inactiveValue;
}

#endif
