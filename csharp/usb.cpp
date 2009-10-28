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

#include <string>

#include "node.h"
#include "usb.h"
#include "util.h"
#include "error.h"

using namespace System;

namespace Nitro {
namespace NET {

	USBDevice::USBDevice(UInt32 vid, UInt32 pid) {
		m_dev=new Nitro::USBDevice(vid,pid);
	}

	Node^ USBDevice::GetTree() {
		try{			
			return gcnew Node(m_dev->get_tree());
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	void USBDevice::SetTree(const Node^ node) {
		try {
			m_dev->set_tree( (Nitro::NodeRef)node );
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	void USBDevice::EnableMode(const DataType^ term, UInt32 mode ) {
		try {
			m_dev->enable_mode(static_cast<Nitro::DataType>(term),mode);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::DisableMode(const DataType^ term, UInt32 mode ) {
		try {
			m_dev->disable_mode(static_cast<Nitro::DataType>(term),mode);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	UInt32 USBDevice::GetModes(const DataType^ term) {
		try {
			return m_dev->get_modes(static_cast<Nitro::DataType>(term));
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::SetModes(const DataType^ term, UInt32 modes) {
		try {
			m_dev->set_modes(static_cast<Nitro::DataType>(term), modes);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	String^ USBDevice::GetDeviceSerial(UInt32 vid, UInt32 pid, UInt32 index) {
		try{
			std::string serial = Nitro::USBDevice::get_device_serial(vid,pid,index);
			return gcnew String(serial.c_str());
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Open() {
		try {
			m_dev->open();
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Open(UInt32 index) {
		try {
			m_dev->open(index);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Open(UInt32 index, Boolean override_version) {
		try {
			m_dev->open(index, override_version);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	void USBDevice::Open(const String^ serial) {
		std::string s=to_string(serial);
		try {
			m_dev->open(s);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	void USBDevice::LoadFirmware(array<Byte>^ bytes ) {
		pin_ptr<Byte> b = &bytes[0];
		try {
			m_dev->load_firmware(reinterpret_cast<char*>(b), bytes->Length );
		} catch ( Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	UInt16 USBDevice::GetFirmwareVersion() {
		try {
			return m_dev->get_firmware_version();
		} catch ( Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	void USBDevice::SetSerial( const String^ serial) {
		std::string ser = to_string(serial);
		try {
			m_dev->set_device_serial(ser);
		} catch ( Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}
	String^ USBDevice::GetSerial () {
		try {
			std::string serial = m_dev->get_device_serial();
			return gcnew String(serial.c_str());
		} catch ( Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
	}


	DataType^ USBDevice::Get(const DataType^ term, const DataType^ reg, UInt32 timeout ) {
		try{
			return  gcnew DataType( (uint32)m_dev->get( 
					static_cast<Nitro::DataType>(term),
					static_cast<Nitro::DataType>(reg), timeout ) ) ;
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Set(const DataType^ term, const DataType^ reg, const DataType^ val, UInt32 timeout) {
		try{
			m_dev->set( 
			 static_cast<Nitro::DataType>(term),
			 static_cast<Nitro::DataType>(reg), 
			 static_cast<Nitro::DataType>(val),
			 timeout );
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Read(const DataType^ term, const DataType^ reg, array<Byte>^ bytes, UInt32 timeout) {
		pin_ptr<Byte> b = &bytes[0];
		try{
			m_dev->read( 
				static_cast<Nitro::DataType>(term),
				static_cast<Nitro::DataType>(reg), 
				static_cast<uint8*>(b),
				bytes->Length,
				timeout );
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Write(const DataType^ term, const DataType^ reg, array<Byte>^ bytes, UInt32 timeout) {
		pin_ptr<Byte> b = &bytes[0];
		try{
			m_dev->write( 
				static_cast<Nitro::DataType>(term),
				static_cast<Nitro::DataType>(reg), 
				static_cast<uint8*>(b),
				bytes->Length,
				timeout );
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}
	void USBDevice::Reset() {
		try{
			m_dev->reset();				
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}

}}
