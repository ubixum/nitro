// Copyright (C) 2017 BrooksEE, LLC 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#ifndef NITRO_VERSIONNO_H
#define NITRO_VERSIONNO_H

#define NITRO_MAJOR 1
#define NITRO_MINOR 3
#define NITRO_RELEASE 0
#define NITRO_SUBREL 1

#define NITRO_VERSION ((uint32)((NITRO_MAJOR << 24)|(NITRO_MINOR << 16)|(NITRO_RELEASE << 8)|NITRO_SUBREL))

#define str_macro(m) #m
#define str_macro_val(m) str_macro(m)
#define NITRO_VERSION_STR \
	str_macro_val(NITRO_MAJOR) "." \
	str_macro_val(NITRO_MINOR) "." \
	str_macro_val(NITRO_RELEASE) "." \
	str_macro_val(NITRO_SUBREL)


#endif
