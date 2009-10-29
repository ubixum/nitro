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


#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <ctime>


#include <vector>

#include <nitro/usb.h>
#include <nitro/error.h>

#include "vendor_commands.h"

#include "ihx.h"


#include "hr_time.h"

#ifdef DEBUG_USB
#define usb_debug(d) (std::cout << d << std::endl)
#else
#define usb_debug(d)
#endif

#define FIRMWARE_MAJOR 1 // current drivers support this version of the device firmware
#define FIRMWARE_MAJOR_MIN 1 // min version drivers support.

#define VC_HI_REGVAL (NITRO_VC)0xb2 // version 1 vendor command no longer in firmware


namespace Nitro {

typedef enum {
 NITRO_IN, // = USB_TYPE_VENDOR|USB_ENDPOINT_IN,
 NITRO_OUT // = USB_TYPE_VENDOR
} NITRO_DIR;

typedef enum {
 READ_EP=0x86,
 WRITE_EP=0x02
} NITRO_EPDEF;

struct usbdev_impl_core {

    int last_transfer_status;
    int last_transfer_checksum;

    usbdev_impl_core() : last_transfer_status(0) {}
    virtual ~usbdev_impl_core() {}

    virtual int control_transfer ( NITRO_DIR, NITRO_VC, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout )=0;
    virtual int bulk_transfer ( NITRO_DIR, uint8, uint8* , size_t, uint32 )=0; 
    int write_ram(uint16 addr, const uint8* data, size_t length, unsigned int timeout ) {
    
    	size_t transferred = 0;
    	// try 1024 bytes at a time.
    	while ( transferred < length ) {
    		int cur_transfer_size = length-transferred > 1024 ? 1024 : length-transferred;
    
            int ret = control_transfer ( NITRO_OUT, VC_RDWR_RAM, addr+transferred, 0, const_cast<uint8*>(data+transferred), cur_transfer_size, timeout );
    		if (ret>0) {
    			transferred += cur_transfer_size;
                usb_debug ( "Transfered " << cur_transfer_size << " to device." );
    		} else {
                throw Exception ( USB_COMM, "Failed to transfer bytes to usb device memory.", ret ); //, usb_strerror() );
            }
                
    	}
    	return length;
    }
   void rdwr_setup (uint8 command, size_t length, uint16 terminal_addr, uint32 reg_addr, uint32 timeout) {
        // send usb read command
        rdwr_data_header c = { command, terminal_addr, reg_addr, length };
    	assert (sizeof(c)==11);
        int ret=control_transfer( NITRO_OUT, VC_HI_RDWR, 0, 0, reinterpret_cast<uint8*>(&c), sizeof(c),  timeout);
        if (ret != sizeof(c)){ 
            throw Exception ( USB_COMM, "Unable to initiate rdwr process on device.", ret);//, usb_strerror() );
        }
    }

    void rdwr_data( NITRO_DIR dir, NITRO_EPDEF ep, uint8* data, size_t length , uint32 timeout ) {
        uint32 transferred=0;
        int tmp_zcount=0;
        usb_debug ( "Transferring " << length << " bytes. Timeout: " << timeout );
        while (transferred<length) {
            #ifdef DEBUG_USB
            CStopWatch timer;
            timer.startTimer();
            #endif
            int ret=bulk_transfer( dir,  ep, data+transferred, length-transferred, timeout);
            #ifdef DEBUG_USB
            timer.stopTimer();
            if (ret>0) {
             double mb = double(ret) / (1024*1024);
             usb_debug ( "Core Transfer Speed " << mb << " MB in " << timer.getElapsedTime() << " s = " << mb/timer.getElapsedTime() << " MBps" ); 
            }
            #endif
            if (ret<0) {
                throw Exception ( USB_COMM, "Unable to write data to device.", ret);//, usb_strerror() );
            }
    
            usb_debug ( "bytes read/written: " << ret );
            
            if (!ret) {
               ++tmp_zcount;
               if (tmp_zcount>3) throw Exception ( USB_COMM, "Unable to Read/Write data to device", "Overflow" ); 
            }
            transferred += ret;
        }
    }
    void read_ack(uint32 timeout) {
           // read bytes for ack
        uint16 ack[4];
        usb_debug ( "Read the ack..." );
        rdwr_data ( NITRO_IN, READ_EP, reinterpret_cast<uint8*>(&ack), sizeof(ack), timeout );
        usb_debug ( "Device Ack: " << ack[0] << ", " << ack[1] << ", " << ack[2] << ", " << ack[3] );
        if (ack[0] != 0xa50f) throw Exception ( USB_COMM, "Invalid transfer ack packet", ack[0] );
        last_transfer_checksum = ack[1];
        last_transfer_status = ack[2];
    }
 
    int toggle_reset ( bool run ) {
    	const uint8 reset = run ? (uint8)0 : (uint8)1;
    	return write_ram ( 0xe600, &reset, 1, 1000 );
    }

};

#ifdef USB_CORE_LIBUSB1
#include "libusb1_impl.cpp"
#else
#include "libusb0_impl.cpp"
#endif

void USBDevice::load_firmware(const char* bytes,size_t length) {

    IhxFile ihx = parse_ihx ( bytes, length );

	// place the device in reset
    m_impl->toggle_reset(false);
    for (IhxFile::iterator itr=ihx.begin(); itr != ihx.end(); ++itr ) {
        m_impl->write_ram(itr->addr,itr->bytes.c_str(),itr->bytes.size(),1000); 
    }
    m_impl->toggle_reset(true);
    m_impl->control_transfer ( NITRO_OUT, VC_RENUM, 0, 0, NULL, 0, 1000); 
    m_impl->close();

}

void USBDevice::renum() {
    m_impl->control_transfer ( NITRO_OUT, VC_RENUM, 0, 0, NULL, 0, 1000 );
    m_impl->close();
}

void USBDevice::reset() {
    m_impl->toggle_reset(false);
    m_impl->toggle_reset(true);
    m_impl->close();
}


USBDevice::USBDevice(uint32 vid, uint32 pid) {
  m_impl=new impl(vid,pid);
}

USBDevice::~USBDevice() throw() {
    m_impl->close();
    delete m_impl;
}

uint32 USBDevice::get_device_count(uint32 vid, uint32 pid) {

  return impl::get_device_count(vid,pid);

}

void USBDevice::open(uint32 index, bool override_version) {

  m_impl->open(index,override_version);
  
}

bool USBDevice::is_open() { return m_impl->is_open(); }

void USBDevice::_close() {
    m_impl->close();
}


DataType USBDevice::_get( uint32 terminal_addr, uint32 reg_addr, uint32 timeout ) {
    
	usb_debug ( "Get term: " << terminal_addr << " reg: " << reg_addr );
    uint16 value=0;
    //_read ( terminal_addr, reg_addr, reinterpret_cast<uint8*>(&value), sizeof(value), timeout );
    m_impl->rdwr_setup ( COMMAND_GET, sizeof(value), terminal_addr, reg_addr, timeout );
    m_impl->rdwr_data ( NITRO_IN, READ_EP, reinterpret_cast<uint8*>(&value), sizeof(value), timeout );
    m_impl->read_ack(timeout);
    usb_debug ( "Value: " << value );
    return DataType( static_cast<uint32>(value) );
}

void USBDevice::_set( uint32 terminal_addr, uint32 reg_addr, const DataType& value, uint32 timeout ) {

	usb_debug ( "Set term: " << terminal_addr << " reg: " << reg_addr << " val: " << value);
    uint16 val = static_cast<uint32>(value);
    //_write ( terminal_addr, reg_addr, reinterpret_cast<uint8*>(&val), sizeof(val), timeout );
    m_impl->rdwr_setup ( COMMAND_SET, sizeof(val), terminal_addr, reg_addr, timeout );
    m_impl->rdwr_data ( NITRO_OUT, WRITE_EP, reinterpret_cast<uint8*>(&val), sizeof(val), timeout );
    m_impl->read_ack(timeout);
}

void USBDevice::_read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {

    // send usb read command
	usb_debug ( "Read term: " << terminal_addr << " reg: " << reg_addr << " length: " << length );
    m_impl->rdwr_setup( COMMAND_READ, length, terminal_addr, reg_addr, timeout );
    m_impl->rdwr_data ( NITRO_IN, READ_EP, data, length, timeout );
    m_impl->read_ack(timeout);

}
void USBDevice::_write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {

	usb_debug ( "Write term: " << terminal_addr << " reg: " << reg_addr << " length: " << length );
    m_impl->rdwr_setup(COMMAND_WRITE, length, terminal_addr, reg_addr, timeout);
	usb_debug ( "Write process initialized." );

    m_impl->rdwr_data( NITRO_OUT, WRITE_EP, const_cast<uint8*>(data), length, timeout );
    m_impl->read_ack(timeout);

}

int USBDevice::_transfer_status() {
    return m_impl->last_transfer_status;
}
uint16 USBDevice::_transfer_checksum() { 
    return m_impl->last_transfer_checksum;
}

void USBDevice::set_device_serial ( const std::string serial ) {
    if ( serial.size() != 8 ) throw Exception ( DEVICE_OP_ERROR, "Invalid serial number length", (uint32)serial.size() ); 
    const char* buf = serial.c_str();
    m_impl->control_transfer ( NITRO_OUT, VC_SERIAL, 0, 0, reinterpret_cast<uint8*>(const_cast<char*>(buf)), 8, 1000 ); 
}

std::string USBDevice::get_device_serial ( ) {
    uint8 buf[8];
    m_impl->control_transfer ( NITRO_IN, VC_SERIAL, 0, 0, buf, 8, 1000 );
    return std::string(reinterpret_cast<const char*>(buf),8);
}

uint16 USBDevice::get_firmware_version() const {
    return m_impl->firmware_version();
}

uint16 USBDevice::get_device_address(uint32 vid, uint32 pid, uint32 index) {
    return impl::get_device_address(vid,pid,index);
}

std::string USBDevice::get_device_serial(uint32 vid, uint32 pid, uint32 index ) {

    return impl::get_device_serial(vid,pid,index);

}
void USBDevice::open(const std::string& serial) {
   
   m_impl->open(serial);
}	

} // end namespace
