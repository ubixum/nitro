/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/


#include <vcclr.h>

#include "util.h"

using namespace System;
using namespace System::Runtime::InteropServices;


std::string to_string( const String^ input ) {
	
	// NOTE vs pro can use marshal_as<std::string> instead
	char* str = (char*)Marshal::StringToHGlobalAnsi(const_cast<String^>(input)).ToPointer();    
	std::string res=str;
	Marshal::FreeHGlobal((IntPtr)str);

	return res;
	
}
