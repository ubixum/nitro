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


#include "types.h"
#include "node.h"


using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

namespace Nitro {
namespace NET {

	public ref class Scripts : public Object {
		private:
			Nitro::Scripts* m_scripts;
		public:
			Scripts();
			~Scripts(){ this->!Scripts(); }
			!Scripts(){ delete m_scripts; }

			void import ( const String^ module, const String^ path );

			DataType^ exec ( const String^ module, const String^ func, Dictionary<String^,DataType^>^ params );

			ArrayList^ func_list ( const String^ module );

			String^ get_comment ( const String^ module, const String^ func );

			Node^ get_params ( const String^ module, const String^ func_name );

	};

}
}
