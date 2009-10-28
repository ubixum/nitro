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

#pragma once

#include <nitro.h>

using namespace System;


namespace Nitro {
namespace NET {
	public ref class NitroException : public System::Exception {
		private:
			int32 m_code;
		public:		
			NitroException(int32 code, String^ err) : System::Exception( err ) {}
			int32 code() { return m_code; }
			~NitroException() {}
			!NitroException() {}
			static explicit operator NitroException^ ( const Nitro::Exception& e );
	};
}}
