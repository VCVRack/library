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

#include "Link.hpp"

Plugin* plugin;

void init(rack::Plugin* p)
{
#ifndef SLUG
    static_assert(false, "SLUG must be defined!");
#endif

#ifndef VERSION
    static_assert(false, "VERSION must be defined!");
#endif

    plugin = p;
    p->slug = TOSTRING(SLUG);
    p->version = TOSTRING(VERSION);

    p->addModel(createModel<LinkWidget>("Stellare Modular", "Link", "Link", CLOCK_TAG));
}
