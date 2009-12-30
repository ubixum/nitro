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

#include <libusb-1.0/libusb.h>

//typedef std::vector<struct usb_device*> DeviceList;
//typedef std::vector<struct usb_device*>::iterator DeviceListItr;


struct USBDevice::impl : public usbdev_impl_core {
    /**
     * Need to initialize once per process (lib load)
     **/

    private:
        static int m_ref_count;
        static bool m_initialized; 
        uint16 m_ver;

        static void check_init();

        libusb_device_handle* m_dev;
        void config_device();
        void check_open() const;

        static uint16 get_addr(libusb_device* dev) {
            uint8 bn = libusb_get_bus_number ( dev );
            uint8 dn = libusb_get_device_address ( dev );
            return (uint16(bn)<<8) | dn;
        }

        class DevItr {
            public:
                virtual bool process(libusb_device*, libusb_device_descriptor &)=0;
                virtual ~DevItr() {}
        };
        class DevCounter: public DevItr {
            public:
                int matches;
                DevCounter() : matches(0) {}
                bool process(libusb_device* dev, libusb_device_descriptor &) {
                    ++matches;
                    return true;
                }
        };
        class DevAddress: public DevItr {
            private:
                uint32 m_curidx;
                uint32 m_idx;
            public:
                uint16 addr;
                DevAddress(uint32 idx) : m_curidx(0), m_idx(idx), addr(0) {}
                bool process(libusb_device* dev, libusb_device_descriptor &) {
                   if ( m_idx == m_curidx++ ) {
                     addr = get_addr(dev); 
                      return false;
                   }
                   return true;
                }
        };
        static void check_ver(uint16 ver) { 
            if (FIRMWARE_MAJOR < (ver>>8) || FIRMWARE_MAJOR_MIN > (ver>>8) )
                throw Exception ( USB_PROTO, "Current drivers don't support firmware version.", ver );
        }
        class IndexOpener : public DevItr {
            private:
                uint32 m_idx;
                uint32 m_curidx;
                bool m_override_version;
            public:
                uint16 m_ver;
                libusb_device_handle* m_dev; 
                IndexOpener ( uint32 idx, bool override_version=false ) : m_idx ( idx ), m_curidx(0), m_override_version(override_version), m_dev(NULL) {} 
                bool process(libusb_device* dev, libusb_device_descriptor& dscr) {
                    if (m_curidx++ == m_idx) {
                        // found device
                        if (!m_override_version) { check_ver(dscr.bcdDevice); }
                        int ret = libusb_open(dev, &m_dev);
                        if (ret) {
                           throw Exception ( USB_PROTO, "Failed to open device.", ret ); 
                        }
                        m_ver=dscr.bcdDevice;
                        return false;
                    }
                    return true; // keep looking
                }
        };

        class AddrOpener : public DevItr {
            private:
                uint16 m_addr;
            public:
                libusb_device_handle* m_dev;
                uint16 m_ver;
                AddrOpener( uint16 addr ) : m_addr(addr), m_dev(NULL) {}
                bool process(libusb_device* dev, libusb_device_descriptor &dscr) {
                    uint16 addr = get_addr(dev); 
                    if (m_addr == addr) {
                        int ret = libusb_open(dev, &m_dev);
                        if (ret) {
                            throw Exception ( USB_PROTO, "Failed to open device.", ret );
                        }
                        m_ver = dscr.bcdDevice;
                        return false;
                        
                    }
                    return true;
                }

        };

        /**
         * Gets the serial number from an opened (not necessarily configured) device.
         **/
        static std::string get_serial(libusb_device_handle* dev) {
            unsigned char *buf = new unsigned char[1025]; // 1k buffer + 1 NULL
            memset(buf,1025,0);
            libusb_device* d = libusb_get_device(dev);
            libusb_device_descriptor dscr;
            int ret = libusb_get_device_descriptor ( d, &dscr );
            std::string res;
            int str_ok=-1;
            if ( !ret ) {
                str_ok = libusb_get_string_descriptor_ascii ( dev, dscr.iSerialNumber, buf, 1024 );
                if ( str_ok >= 0 ) {
                    res = std::string((const char*)buf, str_ok);
                } 
            }
            delete [] buf; 
            if (str_ok >= 0) return res;
            else throw Exception ( USB_PROTO, "Failed to retrieve string descriptor while checking device.", ret );
        }

        class SerialOpener : public DevItr {
            private:
                const std::string &m_serial;
            public:
                uint16 m_ver;
                libusb_device_handle* m_dev; 
                SerialOpener ( const std::string &serial ) : m_serial ( serial ), m_dev(NULL) {} 
                bool process(libusb_device* dev, libusb_device_descriptor& dscr) {

                    int ret = libusb_open(dev,&m_dev);
                    if ( ret ) throw Exception ( USB_PROTO, "Failed opening device while checking serial number." );

                    try {
                        std::string ser = get_serial ( m_dev );
                        m_ver=dscr.bcdDevice;
                        if (ser == m_serial) {
                           check_ver(m_ver); 
                           return false;
                        } else {
                            libusb_close(m_dev);
                            m_dev=NULL;
                            return true;
                        }
                    } catch ( const Exception &e ) {
                        libusb_close(m_dev);
                        throw e;
                    }

                }
        };


        /**
         * iterate through the matching devices, calls DevItr::process for each 
         * matching vid, pid device.
         *
         * itr should return true if it wants to continue processing other 
         * devices or false to stop.
         *
         * only devices matching vid/pid are passed to process method.
         **/
        static void iter_devices ( uint32 vid, uint32 pid, DevItr& itr);
    public:

        impl(uint32 vid, uint32 pid): usbdev_impl_core(vid,pid), m_dev(NULL) { ++m_ref_count; }
        ~impl() { close(); 
            --m_ref_count;
            if (m_initialized && !m_ref_count ) { 
                usb_debug ( "Uninizilizing libusb" );
                libusb_exit(NULL); 
                m_initialized=false;
            }
        }
        void open ( uint32 index, bool override_version=false );
        void open ( const std::string& serial );
        void open_addr ( uint16 addr );
        bool is_open() { return m_dev != NULL; }
        uint16 firmware_version() { check_open(); return m_ver; }
        static uint32 get_device_count ( uint32 vid, uint32 pid );
        static std::string get_device_serial(uint32 vid, uint32 pid, uint32 index );
        static uint16 get_device_address (uint32, uint32, uint32 );
        uint16 get_device_address();
   
        int control_transfer ( NITRO_DIR, NITRO_VC, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout );
        int bulk_transfer ( NITRO_DIR, uint8 ep, uint8* data, size_t length, uint32 timeout ); 
    
        void close();

};

int USBDevice::impl::m_ref_count=0;
bool USBDevice::impl::m_initialized=false;
void USBDevice::impl::check_init() {
    if (!m_initialized) {
        assert ( sizeof(uint8) == 1 );
        assert ( sizeof(uint16) == 2 );
        assert ( sizeof(uint32) == 4);
		assert ( sizeof(uint64) == 8);
    
        int rv=libusb_init(NULL);
        if (rv) {
         throw Exception( USB_INIT );
        }
        m_initialized=true;
		usb_debug ( "Initialized Lib USB1" );
    }
}

void USBDevice::impl::iter_devices ( uint32 vid, uint32 pid, DevItr& itr) {
  check_init();
  libusb_device **list;
  int devices = libusb_get_device_list( NULL, &list );
  usb_debug ( "usb devices to check: " << devices );
  for (int i=0;i<devices;++i) {
     libusb_device_descriptor dscr;
     if (libusb_get_device_descriptor( list[i], &dscr )) {
        libusb_free_device_list ( list, 1 );
        throw Exception ( USB_PROTO, "Error reading device information." );
     }
     if (dscr.idVendor == vid && dscr.idProduct == pid ) {
        try {
            if (!itr.process(list[i], dscr) ) break;
        } catch ( const Exception &e ) {
           libusb_free_device_list(list,1);
           throw e;
        }
     }
  }
  libusb_free_device_list(list,1);
}


uint32 USBDevice::impl::get_device_count(uint32 vid, uint32 pid) {

  DevCounter c;
  iter_devices(vid,pid,c);
  return c.matches;

}

#define CHECK_ALREADY_OPENED() if (m_dev) throw Exception ( USB_PROTO, "Device Already Opened." )

void USBDevice::impl::open(uint32 index, bool override_version) {

 CHECK_ALREADY_OPENED();

 IndexOpener d ( index, override_version );
 iter_devices( m_vid, m_pid, d );
 if (!d.m_dev) throw Exception ( USB_PROTO, "Failed to open device.", "Insufficient devices connected." );
 m_dev = d.m_dev;
 m_ver = d.m_ver;

 config_device();
}

void USBDevice::impl::open_addr(uint16 addr) {
   CHECK_ALREADY_OPENED();
   AddrOpener a ( addr ); 
   iter_devices ( m_vid, m_pid, a );
   if (!a.m_dev) throw Exception ( USB_PROTO, "Failed to open device.", "No matching device address with vid/pid found." );
   m_dev = a.m_dev;
   m_ver = a.m_ver;

   config_device();
}

void USBDevice::impl::open(const std::string& serial) {

 CHECK_ALREADY_OPENED();

 SerialOpener d ( serial );
 iter_devices ( m_vid, m_pid, d );
 if (!d.m_dev) throw Exception ( USB_PROTO, "Failed to open device.", "No device with serial found." );
 m_dev = d.m_dev;
 m_ver = d.m_ver;

 config_device();

}

void USBDevice::impl::check_open() const {
    if (!m_dev) throw Exception(USB_PROTO, "IO method called on unopened device.");
}


void USBDevice::impl::config_device() {

  if (libusb_set_configuration(m_dev,1)) {
    libusb_close(m_dev);
    m_dev=NULL;
    throw Exception(USB_PROTO, "Failed to set device configuration."); //, usb_strerror());
  }
	  
  // claim interface

  if (libusb_claim_interface(m_dev,0)) {
	  libusb_close(m_dev);
      m_dev=NULL;
      throw Exception(USB_PROTO, "Failed to claim device interface."); //, usb_strerror());
  }

  if (libusb_set_interface_alt_setting(m_dev,0,1) ) {
	  libusb_close(m_dev);
	  m_dev=NULL;
	  throw Exception(USB_PROTO, "Failed to set interface alt setting.");
  }
  usb_debug ( "Device configured." );

}
int USBDevice::impl::control_transfer ( NITRO_DIR d, NITRO_VC c, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   int type = d == NITRO_OUT ? 0x40 : 0xc0;
   return libusb_control_transfer( m_dev, type, c, value, index, data, length, timeout ); 
}
int USBDevice::impl::bulk_transfer ( NITRO_DIR d, uint8 ep, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   int transferred=0;
   int rv=libusb_bulk_transfer ( m_dev, ep, data, length, &transferred, timeout );
   if (rv) {
    usb_debug ( "bulk transfer fail: " << rv );
    throw Exception ( USB_COMM, "bulk transfer fail",rv );
   }
   return transferred;
}

void USBDevice::impl::close() {
    if (m_dev) {
      libusb_release_interface(m_dev,0);
      libusb_close(m_dev);
      m_dev=NULL;
	  usb_debug ( "Closed device." );
    }
}

std::string USBDevice::impl::get_device_serial(uint32 vid, uint32 pid, uint32 index ) {
    IndexOpener d ( index );
    iter_devices( vid, pid, d );
    if (!d.m_dev) throw Exception ( USB_PROTO, "Insufficient devices connected." );
    std::string serial;
    try {
        serial=get_serial(d.m_dev); 
    } catch ( const Exception &e) {
        libusb_close(d.m_dev);
        throw e;
    }
    libusb_close(d.m_dev);
    return serial;

}


uint16 USBDevice::impl::get_device_address(uint32 vid, uint32 pid, uint32 index) {

    DevAddress d(index);
    iter_devices(vid,pid,d);
    if (!d.addr) { throw Exception ( USB_COMM, "Invalid device index", index ); }
    return d.addr;

}

uint16 USBDevice::impl::get_device_address() {
    check_open();

    libusb_device* dev = libusb_get_device(m_dev);

    return get_addr(dev);
    
}
