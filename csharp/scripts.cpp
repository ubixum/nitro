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



#include "scripts.h"

#include <string>

#include "util.h"
#include "error.h"

using namespace System::Collections::Generic;
using namespace System::Collections;

namespace Nitro{
namespace NET{

	Scripts::Scripts() {
		try {
			m_scripts = new Nitro::Scripts(); 
		} catch ( const Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}

	void Scripts::import ( const String^ module, const String^ path ) {
		std::string mod = to_string ( module );
		std::string pa = to_string ( path );

		try {
			m_scripts->import(mod, pa);
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	DataType^ Scripts::exec ( const String^ module, const String^ func, Dictionary<String^,DataType^>^ params) {
			
        Nitro::NodeRef n = Nitro::Node::create("params");
       
        pin_ptr<Byte> raw_buffer = nullptr;
        for each ( KeyValuePair<String^, DataType^> kvp in params ) {
			String^ key = kvp.Key;
			DataType^ val = kvp.Value;
			std::string k = to_string(key);
            if (val->GetDataType() == Nitro::BUF_DATA ) {
                array<Byte>^ bytes = val->GetBytes();
                raw_buffer = &bytes[0];

                n->set_attr(k, Nitro::DataType::as_datatype( raw_buffer, bytes->Length ) );
            } else {
			    Nitro::DataType v = static_cast<Nitro::DataType> ( val );
                n->set_attr ( k, v );
            }
			
		}       

		std::string mod = to_string(module);
		std::string f = to_string(func);
		try {
			Nitro::DataType ret = m_scripts->exec ( mod, f, n );
			return gcnew DataType ( ret );
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e); 
		}
	}

	ArrayList^ Scripts::func_list ( const String^ module ) {
		std::string mod = to_string(module);
		try {
			std::vector<std::string> funcs = m_scripts->func_list(mod);
			ArrayList^ ar = gcnew ArrayList();
			for ( std::vector<std::string>::iterator itr = funcs.begin(); itr != funcs.end(); ++itr ) {
				std::string func = *itr;
				String^ f = gcnew String( func.c_str() );
				ar->Add ( f );
			}
			return ar;
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	String^ Scripts::get_comment ( const String^ module, const String^ func ) {
		std::string mod = to_string(module);
		std::string f = to_string(func);
		try {
			std::string comment = m_scripts->get_comment(mod,f);
			return gcnew String ( comment.c_str() );
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	Node^ Scripts::get_params ( const String^ module, const String^ func_name ) {
		std::string mod = to_string(module);
		std::string f = to_string(func_name);
		try {
			NodeRef params = m_scripts->get_params(mod,f);
			return gcnew Node(params);
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

}
}
