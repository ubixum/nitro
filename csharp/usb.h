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

#include <nitro.h>


namespace Nitro {
namespace NET {	

	ref class Node;
	ref class DataType;

	public ref class USBDevice : public Object {
	private:
		Nitro::USBDevice* m_dev;
	public:

		literal UInt32 GETSET_VERIFY = Nitro::Device::GETSET_VERIFY;
		literal UInt32 DOUBLEGET_VERIFY = Nitro::Device::DOUBLEGET_VERIFY;
		literal UInt32 STATUS_VERIFY = Nitro::Device::STATUS_VERIFY;
		literal UInt32 CHECKSUM_VERIFY = Nitro::Device::CHECKSUM_VERIFY;
		literal UInt32 RETRY_ON_FAILURE = Nitro::Device::RETRY_ON_FAILURE;


		USBDevice( UInt32 vid, UInt32 pid );
		~USBDevice() { this->!USBDevice(); }
        !USBDevice() { m_dev->close(); delete m_dev; }
		static UInt32 GetDeviceCount (UInt32 vid, UInt32 pid) { return Nitro::USBDevice::get_device_count(vid,pid); }
		static String^ GetDeviceSerial(UInt32 vid, UInt32 pid, UInt32 index);
		void Open();
		void Open(UInt32 index );
		void Open(UInt32 index, Boolean override_version);
		void Open(const String^ serial);
		void LoadFirmware( array<Byte>^ bytes );
		UInt16 GetFirmwareVersion() ;

		void SetSerial( const String^ );
		String^ GetSerial () ;


        void Lock() { m_dev->lock(); }
        void Unlock() { m_dev->unlock(); }
		void SetTimeout(UInt32 t) { m_dev->set_timeout(t); }
		UInt32 GetTimeout() { return m_dev->get_timeout(); }

		void EnableMode(UInt32 mode) { m_dev->enable_mode(mode); }
		void DisableMode(UInt32 mode) { m_dev->disable_mode(mode); }
		void EnableMode(const DataType^ term, UInt32 mode );
		void DisableMode(const DataType^ term, UInt32 mode );
		UInt32 GetModes() { return m_dev->get_modes(); }
		UInt32 GetModes(const DataType^ term);
		void SetModes(UInt32 modes) { m_dev->set_modes(modes); }
		void SetModes(const DataType^ term, UInt32 modes);

		DataType^ Get(const DataType^ term, const DataType^ reg, UInt32 timeout );
		void Set(const DataType^ term, const DataType^ reg, const DataType^ val, UInt32 timeout);
		void Read(const DataType^ term, const DataType^ reg, array<Byte>^ bytes, UInt32 timeout);
		void Write(const DataType^ term, const DataType^ reg, array<Byte>^ bytes, UInt32 timeout);
		Node^ GetTree ();
		void SetTree(const Node^ node);
		void Reset();
		void Close () { m_dev->close(); }
	
		Nitro::USBDevice* get_dev() { return m_dev; }
	};
	

}}
