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

#ifndef LINKWRAPPER_H
#define LINKWRAPPER_H

#ifdef LINK_WRAPPER_DLL
#define EXPORT_ATTRIBUTES __declspec(dllexport)
#else
#define EXPORT_ATTRIBUTES
#endif // LINK_WRAPPER_DLL

#include<stddef.h>

extern "C" {

/* Link opaque handle */
typedef size_t link_handle;

/* Minimalistic Link API */
EXPORT_ATTRIBUTES link_handle* create_link_wrapper();
EXPORT_ATTRIBUTES void delete_link_wrapper(link_handle*);
EXPORT_ATTRIBUTES double get_link_phase(link_handle*, double);

} // extern "C"

#endif  // LINKWRAPPER_H
