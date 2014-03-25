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
#define usb_debug(d) (std::cout << __FILE__ << ':' << __LINE__ << ' ' << d << std::endl)
#else
#define usb_debug(d)
#endif

#define FIRMWARE_MAJOR 3 // current drivers support this version of the device firmware
#define FIRMWARE_MAJOR_MIN 0 // min version drivers support.

#define VC_HI_REGVAL (NITRO_VC)0xb2 // version 1 vendor command no longer in firmware

// protocol v2 structure
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct {
    uint8 command;
    uint32 length;
}
#ifdef __GNUG__
 __attribute__((__packed__))
#endif
rdwr_v2;
#ifdef WIN32
#pragma pack(pop)
#endif

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

    uint32 m_vid;
    uint32 m_pid;

    int last_transfer_status;
    int last_transfer_checksum;

    usbdev_impl_core(uint32 vid, uint32 pid) : m_vid(vid), m_pid(pid), last_transfer_status(0) {}
    virtual ~usbdev_impl_core() {}

    virtual int control_transfer ( NITRO_DIR, NITRO_VC, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout )=0;
    virtual int bulk_transfer ( NITRO_DIR, uint8, uint8* , size_t, uint32 )=0; 
    virtual uint16 firmware_version() = 0;
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
        if ( (firmware_version() >> 8) < 3) {
	  rdwr_v2 c = { command, (uint32) length };
            assert (sizeof(c)==5);
            int ret=control_transfer( NITRO_OUT, VC_HI_RDWR, terminal_addr, reg_addr, reinterpret_cast<uint8*>(&c), sizeof(c),  timeout);
            if (ret != sizeof(c)){ 
                throw Exception ( USB_COMM, "Unable to initiate rdwr process on device.", ret);//, usb_strerror() );
            }
        } else {
	  rdwr_data_header c = { command, terminal_addr, reg_addr, (uint32) length };
        	assert (sizeof(c)==11);
            // NOTE length will be truncated to 16 bits, but for 
            // purposes of fx3 we only need if it's divisible by 4.
            int ret=control_transfer( NITRO_OUT, VC_HI_RDWR, terminal_addr, length, reinterpret_cast<uint8*>(&c), sizeof(c),  timeout);
            if (ret != sizeof(c)){ 
                throw Exception ( USB_COMM, "Unable to initiate rdwr process on device.", ret);//, usb_strerror() );
            }
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
    
            usb_debug ( "bytes read/written: " << ret << " (total: " << (transferred+ret) << ")" );
            
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


uint32 USBDevice::get_vid() {
    return m_impl->m_vid;
}
uint32 USBDevice::get_pid() {
    return m_impl->m_pid;
}

void USBDevice::load_firmware(const char* bytes,size_t length) {
  if ( strncmp(bytes, "CY", 2)==0) {
    // check if this is fx3
    load_fx3_firmware(bytes, length);
    return;
  }

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

std::vector<std::vector<int> > USBDevice::get_device_list(int vid, int pid) {

  return impl::get_device_list(vid, pid);

}

void USBDevice::open(uint32 index, bool override_version) {

  m_impl->open(index,override_version);
  
}

void USBDevice::open_by_address ( uint16 addr ) {
    m_impl->open_addr ( addr );
}

bool USBDevice::is_open() { return m_impl->is_open(); }

void USBDevice::_close() {
    m_impl->close();
}


void USBDevice::_read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {

    // send usb read command
	usb_debug ( "Read term: " << terminal_addr << " reg: " << reg_addr << " length: " << length );
    m_impl->rdwr_setup( COMMAND_READ, length, terminal_addr, reg_addr, timeout );
  

    m_impl->rdwr_data ( NITRO_IN, READ_EP, data, length, timeout );

    if ((m_impl->firmware_version() >> 8) >= 2) {
        m_impl->read_ack(timeout);
    }

}

// old structure for 1.0 firmware
// structure for passing data from rdwr vendor command to rdwr handlers
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct { 
  uint8 in_progress;
  uint8 initialized;
  uint8 command;
  uint16 term_addr;
  uint16 reg_addr;
  uint32 transfer_length;
  uint16 bytes_avail; // bytes available for writing or ep size for reading
  union {
   uint32 bytes_written;
   uint32 bytes_read;
  };
  uint8 aborted;
  uint32 busy_cnt;
  uint32 gpif_tc;
  uint8 gpif_idlecs;
  uint32 buffer_full;

} 
#ifdef __GNUG__
 __attribute__((__packed__))
#endif
rdwr_v1;
#ifdef WIN32
#pragma pack(pop)
#endif


void USBDevice::_write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {

	usb_debug ( "Write term: " << terminal_addr << " reg: " << reg_addr << " length: " << length );
    m_impl->rdwr_setup(COMMAND_WRITE, length, terminal_addr, reg_addr, timeout);
	usb_debug ( "Write process initialized." );

	//	usleep(1000);
    m_impl->rdwr_data( NITRO_OUT, WRITE_EP, const_cast<uint8*>(data), length, timeout );

    if ((m_impl->firmware_version() >> 8) < 2) {
        // when a write occurs, you have to wait for the device to finish writing.
        rdwr_v1 stats;
        int sanity_check=0;
        do {
            usb_debug ( "Transfer finished, checking device done." );
            m_impl->control_transfer ( NITRO_IN, VC_RDWR_STAT, 0, 0, reinterpret_cast<uint8*>(&stats), sizeof(stats), 10000 ); 
            usb_debug ( "Transfer Length " << stats.transfer_length << " bytes written " << stats.bytes_written );
        } while ( stats.bytes_written < stats.transfer_length && sanity_check++<10);
    } else {
        m_impl->read_ack(timeout);
    }

}

int USBDevice::_transfer_status() {
    return m_impl->last_transfer_status;
}
uint16 USBDevice::_transfer_checksum() { 
    return m_impl->last_transfer_checksum;
}

void USBDevice::set_device_serial ( const std::string serial ) {

    if ((m_impl->firmware_version() >> 8) < 2) throw Exception ( DEVICE_OP_ERROR, "Firmware version older than 3.0 does not support this method." );
    if ( serial.size() != 8 ) throw Exception ( DEVICE_OP_ERROR, "Invalid serial number length", (uint32)serial.size() ); 

    if ((m_impl->firmware_version() >> 8) > 2) {
        std::wstring ser;
        ser.assign(serial.begin(),serial.end());
        set_device_serial(ser);
    } else {
        const char* buf=serial.c_str();
        int ret = m_impl->control_transfer ( NITRO_OUT, VC_SERIAL, 0, 0, reinterpret_cast<uint8*>(const_cast<char*>(buf)), 8, 1000 );
        if (ret < 0) throw Exception ( USB_COMM, "Set Serial Failed." );
    } 

}

void USBDevice::set_device_serial ( const std::wstring serial ) {
    if ((m_impl->firmware_version() >> 8) < 3) throw Exception ( DEVICE_OP_ERROR, "Firmware version older than 3.0 does not support this method." );
    if ( serial.size() != 8 ) throw Exception ( DEVICE_OP_ERROR, "Invalid serial number length", (uint32)serial.size() ); 
    uint16 buf[8];
    for (int i=0;i<8;++i) { // have to copy because size of wchar_t might be 4 instead of 2
        buf[i] = serial[i];
    }
    int ret = m_impl->control_transfer ( NITRO_OUT, VC_SERIAL, 0, 0, reinterpret_cast<uint8*>(buf), 16, 1000 );
    if (ret < 0) throw Exception ( USB_COMM, "Set Serial Failed." );
}

std::wstring USBDevice::get_device_serial ( ) {
    if ((m_impl->firmware_version() >> 8) < 2) throw Exception ( DEVICE_OP_ERROR, "Firmware version older than 2.0 does not support this method." );
    int buflen = (m_impl->firmware_version() >> 8) > 2 ?
                 16 : // unicode for version 3+
                 8; // ascii before that
    uint8 buf[16]; // the max
    int ret = m_impl->control_transfer ( NITRO_IN, VC_SERIAL, 0, 0, buf, buflen, 1000 );
    if (ret<0) throw Exception ( USB_COMM, "Get Serial Failed." );
    std::wstring serial;
    // use copy instead because wchar_t is 32 instead of 16 on some platforms
    for (int i=0;i<8;++i) {
        if (8==buflen) {
            serial.append ( 1, buf[i] );
        } else {
            serial.append ( 1, ((uint16*)buf)[i] );
        }
    }
    return serial;
}

uint16 USBDevice::get_ver() const {
    return m_impl->firmware_version();
}

uint16 USBDevice::get_device_address(uint32 vid, uint32 pid, uint32 index) {
    return impl::get_device_address(vid,pid,index);
}

uint16 USBDevice::get_device_address() {
    return m_impl->get_device_address();
}


std::wstring USBDevice::get_device_serial(uint32 vid, uint32 pid, uint32 index ) {

    return impl::get_device_serial(vid,pid,index);

}
void USBDevice::open(const std::string& serial) {
   
   m_impl->open(serial);
}	

void USBDevice::open(const std::wstring& serial) {
   m_impl->open(serial);
}





size_t USBDevice::write_fx3_ram(uint32 addr, const uint8* data, size_t length, unsigned int timeout ) {
  size_t transferred = 0;
  uint32 addr1;
  // try 4096 bytes at a time.
  while ( transferred < length ) {
    int cur_transfer_size = length-transferred > 4096 ? 4096 : length-transferred;
    addr1 = addr + transferred;
    int ret = m_impl->control_transfer ( NITRO_OUT, VC_RDWR_RAM, addr1 & 0xFFFF, addr1 >> 16, const_cast<uint8*>(data+transferred), cur_transfer_size, timeout );
    if (ret>0) {
      transferred += cur_transfer_size;
      usb_debug ( "Transfered " << cur_transfer_size << " to device." );
    } else {
      throw Exception ( USB_COMM, "Failed to transfer bytes to usb device memory.", ret ); //, usb_strerror() );
    }
  }
  return length;
}


void USBDevice::load_fx3_firmware(const char* bytes, size_t length) {
  uint8 *buf = (uint8 *) bytes;
  uint32 dlen, address, program_entry;
  int r;

  usb_debug("Loading firmware FX3**************");

  /* Check first 2 bytes, must be equal to 'CY'	*/
  if ( strncmp((char *) buf, "CY", 2) ) {
    throw Exception ( USB_FIRMWARE, "Image does not have 'CY' at start. Aborting.", -2);
  }
  buf += 2; // advance pointer

  /* Read 1 byte. bImageCTL	*/
  if ( buf[0] & 0x01 ) {
    throw Exception ( USB_FIRMWARE, "Image does not contain executable code.", -3);
  }
  buf += 1;

  /* Read 1 byte. bImageType	*/
  if ( !(buf[0] == 0xB0) ) {
    throw Exception (USB_FIRMWARE, "Not a normal FW binary with checksum", -4);
  }
  buf += 1;

  while (1) {
    /* Read Length of section 1,2,3, ...	*/
    dlen = *((uint32 *) buf);
    buf += 4;
    /* Read Address of section 1,2,3,...	*/
    address = *((uint32 *)buf);   
    buf += 4;
    if ( dlen != 0 ) {
      /* Write data bytes */
      write_fx3_ram(address, (uint8 *) buf, dlen*4, 1000);
      buf += dlen*4;

//      for ( j = 0; j < len/4; ++j )
//	checksum += pint[j];

    } else {
      program_entry = address;
      break;
    }
  }
//  /* Read checksum	*/
//  file_checksum = *((uint32 *)buf);
//  if ( *pdbuf != checksum ) {
//    printf("Error in checksum\n");
//    return -5;
//  }
  nitro_sleep(1e6); // 1 second
  // write the program entry point
  r = m_impl->control_transfer(NITRO_OUT, VC_RDWR_RAM, (program_entry & 0x0000ffff ) , program_entry >> 16, NULL, 0, 1000);
  if ( r ) {
    throw Exception (USB_COMM, "Error in control_transfer", r);
  }
  m_impl->close();
}

} // end namespace
