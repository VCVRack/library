/*

VCV-Link by Stellare Modular
Copyright (C) 2017 - Vincenzo Pietropaolo, Sander Baan

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

#include "link-wrapper.h"

#include <ableton/Link.hpp>

extern "C" {

EXPORT_ATTRIBUTES link_handle* create_link_wrapper()
{
	ableton::Link* link = new ableton::Link(120.0);
	link->enable(true);

	return (link_handle*) link;
}

EXPORT_ATTRIBUTES void delete_link_wrapper(link_handle* handle)
{
	ableton::Link* link = (ableton::Link*) handle;

	if (link)
	{
		link->enable(false);
		delete link;
	}
}

EXPORT_ATTRIBUTES double get_link_phase(link_handle* handle, double quantum)
{
	double phase = 0.0;
	ableton::Link* link = (ableton::Link*) handle;

	if (link)
	{
		const auto time = link->clock().micros();
		const auto timeline = link->captureAppTimeline();
		phase = timeline.phaseAtTime(time, quantum);
	}

	return phase;
}

} // extern "C"
