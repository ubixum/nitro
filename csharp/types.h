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


using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

#include <nitro.h>

#include "error.h"
#include "util.h"
#include "usb.h"
#include "node.h"

namespace Nitro {
namespace NET {

	

	public ref class DataType : public Object
	{		
	private:
		Nitro::DataType *m_dataType;
        array<Byte>^ m_bytes; // for buffers
	public:
		DataType(Int32 val); // int_data
		DataType(UInt32 val); // uint_data 
		DataType(const System::String^ val); // str_data
        DataType(Double); // float_data
        DataType(const Node^ node); // node data
        DataType(List<UInt32>^ val); // bigint data
        DataType(List<DataType^>^ val); // list data        
        DataType(USBDevice^ dev); // dev data
        DataType(array<Byte>^); // buf data

        // copy constructors
		DataType(const DataType^ copy); 
		DataType(const Nitro::DataType& copy);
        
        // destructor/finalizer
		~DataType() { this->!DataType(); }
		!DataType() { delete m_dataType; }

        /**
         * \brief get buf_data bytes
         * 
         * If type if BUF_DATA, then this function returns the byte array
         **/
        array<Byte>^ GetBytes() { return m_bytes; }

		/**
		 * 
		 **/
		virtual bool Equals(Object^ obj) override;		

		const String^ GetDataTypeString() { return gcnew String( m_dataType->str_type().c_str() ); }
		/**
		 * Replace str_value with ToString
		 **/
		virtual String^ ToString() override { return gcnew String( m_dataType->str_value().c_str() ); }
		Int32 GetDataType() { return m_dataType->get_type(); }


        /**
         * Don't allow C++ to cast to buffer datatype.
         * Instead, C++ functions must explicitly call getBytes and pin the byte pointer for the
         * durration of the function.
         **/
		static explicit operator Nitro::DataType& ( const DataType^ value ) { 
            if (value->m_dataType->get_type() == Nitro::BUF_DATA)
                throw gcnew System::Exception ( "Unsupported cast from buffer type to Nitro::DataType");
            return *(value->m_dataType); 
        }

		// cast DataType to values
        // int
		static operator Int32 ( const DataType^ value) { return static_cast<int32>(*(value->m_dataType)); }
        // uint
        static operator UInt32 ( const DataType^ value) { return static_cast<uint32>(*(value->m_dataType)); }
        // bigint
        static operator List<UInt32>^ ( const DataType^ value ) { 
            try {
                std::vector<Nitro::DataType> ints = Nitro::DataType::as_bigints( static_cast<Nitro::DataType>(value) );
                List<UInt32>^ l = gcnew List<UInt32>();                
                for each ( UInt32 i in ints ) {                    
                   l->Add(i);
                }
                return l;
            } catch ( const Nitro::Exception &e ) {
                throw static_cast<NitroException^>(e);
            }
        }
        // str
        static operator String^ ( const DataType^ value) { 
            try {
                std::string str = (std::string)(Nitro::DataType)value;
                return gcnew String(str.c_str());
            } catch ( const Nitro::Exception &e) {
                throw static_cast<NitroException^>(e);
            }
        }
        // float
        static operator Double ( const DataType^ value) { return static_cast<double>(*(value->m_dataType)); }
        // node data is cast as a dictionary
        static operator Node^ ( const DataType^ value);
        // list
        static operator List<DataType^>^ ( const DataType^ value ) {
            try {
                std::vector<Nitro::DataType> v = static_cast<Nitro::DataType>(value);
                List<DataType^>^ l = gcnew List<DataType^>();
                for each ( Nitro::DataType dt in v ) {
                    l->Add ( gcnew DataType ( dt ) );
                }
                return l;
            } catch ( const Nitro::Exception &e) {
                throw static_cast<NitroException^>(e);
            }
        }
		

		// cast values to DataType
		static operator DataType^ ( Int32 val ) { return gcnew DataType(val); }
		static operator DataType^ ( UInt32 val ) { return gcnew DataType(val); }
        static operator DataType^ ( List<UInt32>^ val ) { return gcnew DataType(val); }
		static operator DataType^ ( const String^ val ) { return gcnew DataType(val); }	
        static operator DataType^ ( Double val ) { return gcnew DataType ( val ); }
        static operator DataType^ ( Node^ val ) { return gcnew DataType( val ); }
        static operator DataType^ ( List<DataType^>^ val ) { return gcnew DataType ( val ); }
		static operator DataType^ ( USBDevice^ val ) { return gcnew DataType(val); }
		static operator DataType^ ( array<Byte>^ val ) { return gcnew DataType(val); }

	};

}} // end Nitro::Managed
