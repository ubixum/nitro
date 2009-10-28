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

// This is the main DLL file.

#include <string>

#include "types.h"
#include "util.h"

namespace Nitro {
namespace NET {

// int
	DataType::DataType(Int32 val) {
		m_dataType=new Nitro::DataType(val);
	}
// uint
	DataType::DataType(UInt32 val) {
		m_dataType=new Nitro::DataType(val);
	}
// bigint

    DataType::DataType(List<UInt32>^ val) {
        std::vector<Nitro::DataType> v;
        for each ( UInt32 i in val ) { v.push_back(i); }
        m_dataType=new Nitro::DataType(0);
        *m_dataType = Nitro::DataType::as_bigint_datatype(v);
    }

// float
    DataType::DataType(Double val) {
        m_dataType=new Nitro::DataType(val);
    }

// str
	DataType::DataType(const String^ val) {
		
		std::string val_str = to_string(val);
		m_dataType=new Nitro::DataType( val_str );
	}

// node
	DataType::DataType(const Node^ node) {
		m_dataType=new Nitro::DataType(0);
		*m_dataType = static_cast<Nitro::NodeRef>(node);
	}

// list
    DataType::DataType(List<DataType^>^ list) {
        std::vector<Nitro::DataType> v;
        for each ( DataType^ d in list ) {
            Nitro::DataType dt = static_cast<Nitro::DataType>(d);
            v.push_back(dt);
        }
        m_dataType = new Nitro::DataType(v);
    }

// dev
    DataType::DataType(USBDevice^ dev) {
        m_dataType=new Nitro::DataType(*(dev->get_dev()));
        
    }


// buf

	DataType::DataType(array<Byte>^ bytes) {		
		this->m_bytes = bytes;
		m_dataType = new Nitro::DataType(0); 
		*m_dataType = Nitro::DataType::as_datatype( NULL, bytes->Length );
	}

	DataType::DataType(const DataType ^copy) {
		m_dataType=new Nitro::DataType ( *(copy->m_dataType) );
	}

	DataType::DataType(const Nitro::DataType& copy) {
		m_dataType=new Nitro::DataType ( copy );
	}

    // for some reason I don't know, putting
    // this function defined in the header doesn't 
    // allow Node(NodeRef) to be called
    DataType::operator Node^ ( const DataType^ value ) 
   {			
		try {
            Nitro::DataType& dt = static_cast<Nitro::DataType>(value);
			Nitro::NodeRef n = (Nitro::NodeRef)dt;                

            return static_cast<Node^>(n);
            //return nullptr;
		} catch ( const Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}

	}


	bool DataType::Equals(Object^ obj) { 
		if (obj->GetType() == Int32::typeid) {
			int32 val=static_cast<int32>(obj);
			return *m_dataType == val;
		}
		if (obj->GetType() == UInt32::typeid) {
			uint32 val=static_cast<uint32>(obj);
			return *m_dataType == val;
		}
		if (obj->GetType() == String::typeid) {
			std::string val = to_string( static_cast<String^>(obj) );
			return *m_dataType == val;
		}
		if (obj->GetType() == this->GetType()) {
			return *m_dataType == *(static_cast<DataType^>(obj)->m_dataType);
		}
		
		return false;
	}
		

}} // end Nitro::Managed
