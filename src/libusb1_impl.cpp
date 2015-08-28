/**
 * Copyright (C) 2015 BrooksEE, Inc. 
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

#ifdef WIN32
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#include <queue>

//typedef std::vector<struct usb_device*> DeviceList;
//typedef std::vector<struct usb_device*>::iterator DeviceListItr;

namespace Nitro {

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
        uint8 m_read_ep;
        uint8 m_write_ep;
        uint8 m_interface;
        uint8 m_altsetting;

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
        static std::wstring get_serial(libusb_device_handle* dev) {
            unsigned char *buf = new unsigned char[1025]; // 1k buffer + 1 NULL
            memset(buf,1025,0);
            libusb_device* d = libusb_get_device(dev);
            libusb_device_descriptor dscr;
            int ret = libusb_get_device_descriptor ( d, &dscr );
            std::wstring res;
            int str_ok=-1;
            if ( !ret ) {
                str_ok = libusb_get_string_descriptor( dev, dscr.iSerialNumber, 0x0409, buf, 1024 );
                for (int i=0;i<8;++i) {
                    res.append ( 1, (wchar_t)(((uint16*)buf + 1)[i])) ;
                }
                //if ( str_ok >= 0 ) {
                //    for (int i=0;i<str_ok;++i)
                //        res.append ( 1, ((uint16*)buf)[i] );
                //} 
            }
            delete [] buf; 
            if (str_ok >= 0) return res;
            else throw Exception ( USB_PROTO, "Failed to retrieve string descriptor while checking device.", ret );
        }

        class SerialOpener : public DevItr {
            private:
                const std::wstring &m_serial;
            public:
                uint16 m_ver;
                libusb_device_handle* m_dev; 
                SerialOpener ( const std::wstring &serial ) : m_serial ( serial ), m_dev(NULL) {} 
                bool process(libusb_device* dev, libusb_device_descriptor& dscr) {

                    int ret = libusb_open(dev,&m_dev);
                    if ( ret ) throw Exception ( USB_PROTO, "Failed opening device while checking serial number." );

                    try {
                        std::wstring ser = get_serial ( m_dev );
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
        void open ( const std::wstring& serial );
        void open_addr ( uint16 addr );
        bool is_open() { return m_dev != NULL; }
        uint16 firmware_version() { check_open(); return m_ver; }
        static uint32 get_device_count ( uint32 vid, uint32 pid );
    static std::vector<std::vector<int> > get_device_list (int vid=-1, int pid=-1);
        static std::wstring get_device_serial(uint32 vid, uint32 pid, uint32 index );
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
        #ifdef DEBUG_USB
        libusb_set_debug(NULL,1);
        #endif
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

std::vector<std::vector<int> > USBDevice::impl::get_device_list(int vid, int pid) {
  check_init();
  libusb_device **list;
  int devices = libusb_get_device_list( NULL, &list );
  usb_debug ( "usb devices to check: " << devices );
  std::vector<std::vector<int> > vec;
  for (int i=0;i<devices;++i) {
     libusb_device_descriptor dscr;
     if (libusb_get_device_descriptor( list[i], &dscr )) {
        libusb_free_device_list ( list, 1 );
        throw Exception ( USB_PROTO, "Error reading device information." );
     }
     if ((vid<0 || dscr.idVendor  == (uint32) vid) &&
	 (pid<0 || dscr.idProduct == (uint32) pid)) {
	 std::vector<int> element(3);
	 element[0] = dscr.idVendor;
	 element[1] = dscr.idProduct;
	 element[2] = get_addr(list[i]);
	 vec.push_back(element);
     }
  }
  libusb_free_device_list(list,1);
  return vec;
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
    std::wstring ser;
    ser.assign( serial.begin(), serial.end() );
    open(ser);
}

void USBDevice::impl::open(const std::wstring& serial) {

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
  libusb_device *dev;
  libusb_config_descriptor *config;
  const libusb_interface *iface;
  const libusb_interface_descriptor *idesc;
  const libusb_endpoint_descriptor *epdesc;

  if (libusb_set_configuration(m_dev,1)) {
    libusb_close(m_dev);
    m_dev=NULL;
    throw Exception(USB_PROTO, "Failed to set device configuration."); //, usb_strerror());
  }
	  
  // Find the altsetting and ep addresses
  dev = libusb_get_device(m_dev);
  if(libusb_get_active_config_descriptor(dev, &config)) {
    libusb_close(m_dev);
    m_dev=NULL;
    throw Exception(USB_PROTO, "Failed to get active device configuration.");
  }
  m_read_ep = m_write_ep = m_altsetting = 0;
  m_interface = 0; // claim the first if there isn't any that match w/ 2 endpoints 
                   // useful for programming firmware etc.
  for(unsigned char i=0; i < config->bNumInterfaces; i++) {
    iface = config->interface + i;
    for(int j=0; j < iface->num_altsetting; j++) {
      idesc = iface->altsetting + j;
      usb_debug(" Interface=" << (int) i << " altsetting=" << j << " num_ep=" << (int) idesc->bNumEndpoints);
      if(idesc->bNumEndpoints == 2) {
	for(unsigned char k=0; k<idesc->bNumEndpoints; k++) {
	  epdesc = idesc->endpoint + k;
	  usb_debug("  EP addr = " << (int) epdesc->bEndpointAddress);
	  if(epdesc->bEndpointAddress & 0x80) {
	    m_read_ep = epdesc->bEndpointAddress;
	    usb_debug(" setting m_read_ep=" << (int) epdesc->bEndpointAddress);
	  } else {
	    m_write_ep = epdesc->bEndpointAddress;
	    usb_debug(" setting m_write_ep=" << (int) epdesc->bEndpointAddress);
	  }
	}
      m_interface = idesc->bInterfaceNumber;
	  m_altsetting = idesc->bAlternateSetting;
      }
    }
  }
  libusb_free_config_descriptor(config);
  usb_debug("m_interface="<<(int)m_interface<< " m_read_ep="<< (int) m_read_ep << " m_write_ep="<< (int) m_write_ep << " altsetting=" << (int) m_altsetting);

  //if(m_read_ep == 0 || m_write_ep==0) {
  //  libusb_close(m_dev);
  //  m_dev=NULL;
  //  throw Exception(USB_PROTO, "Failed to find the read and write endpoint.");
  //}

  // claim interface
  if (libusb_claim_interface(m_dev,m_interface)) {
	  libusb_close(m_dev);
      m_dev=NULL;
      throw Exception(USB_PROTO, "Failed to claim device interface."); //, usb_strerror());
  }

  if(m_altsetting) {
    if (libusb_set_interface_alt_setting(m_dev,m_interface,m_altsetting) ) {
  	  libusb_close(m_dev);
  	  m_dev=NULL;
  	  throw Exception(USB_PROTO, "Failed to set interface alt setting.");
    }
  }
  usb_debug ( "Device configured." );

}
int USBDevice::impl::control_transfer ( NITRO_DIR d, NITRO_VC c, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   int type = d == NITRO_OUT ? 0x40 : 0xc0;
   return libusb_control_transfer( m_dev, type, c, value, index, data, length, timeout ); 
}

#define NITRO_TX_SIZE  (64*1024) // seemed to be the fastest buffer size
#define NITRO_TX_QUEUE_DEPTH 32 

typedef struct {
   libusb_device_handle *dev;
   uint8_t ep;
   std::vector<libusb_transfer*> transfers;
   int length;
   uint8_t* data;
   int queued;
   int transferred;
   int timeout;
   int err;

} usb_async_tx_struct;


void usb_tx_submit_helper(usb_async_tx_struct *tx_struct, libusb_transfer *tx);

void usb_tx_callback(libusb_transfer *tx) {
    
    usb_async_tx_struct * tx_struct = (usb_async_tx_struct*)tx->user_data;

    if (tx_struct->transfers.size()<1) {
        usb_debug ( "bad logic no tranfers in transfer queue." ); 
        tx_struct->err = USB_PROTO;
        libusb_free_transfer(tx);
        return;
    }

    libusb_transfer *f = tx_struct->transfers.front();
    tx_struct->transfers.erase(tx_struct->transfers.begin());

    if (f != tx) {
        // this should never happen
        usb_debug ( "bad async request order, callback tx not first in queue." );
        usb_debug ( "front " << f << " tx " << tx );
        tx_struct->err = USB_PROTO; 
        // possible memory leak here but who to free? 
        // fix submission or libusb error instead.
    } else {
        if (tx->status != LIBUSB_TRANSFER_COMPLETED) {
            tx_struct->err = tx->status;
            usb_debug ( "tx packet failed " << tx->status );
        } else if (tx->actual_length < tx->length) {
            tx_struct->err = USB_COMM; 
            usb_debug ( "tx packet transferred less than requested. " );
        }
    }

    if (tx_struct->err) {
        libusb_free_transfer( tx );
    } else {
        tx_struct->transferred += tx->actual_length;
        usb_tx_submit_helper(tx_struct, tx); // resubmit or free
    }

}

void usb_tx_submit_helper(usb_async_tx_struct *tx_struct, libusb_transfer *tx) {

    if (tx_struct->queued >= tx_struct->length) {
        // in this case, we're done queuing, free the tx
        if (tx) libusb_free_transfer(tx);
        return;
    }

    //libusb_transfer *tx = tx ? libusb_alloc_transfer(0) : 
    if (!tx) tx = libusb_alloc_transfer(0);
    int this_len = tx_struct->queued + NITRO_TX_SIZE > tx_struct->length ? tx_struct->length-tx_struct->queued : NITRO_TX_SIZE;
    libusb_fill_bulk_transfer(
       tx,
       tx_struct->dev,
       tx_struct->ep,
       (tx_struct->data + tx_struct->queued),
       this_len, 
       usb_tx_callback,
       tx_struct, 
       tx_struct->timeout );
    tx_struct->transfers.push_back(tx);
    tx_struct->queued += this_len;
    libusb_submit_transfer(tx);
}


int USBDevice::impl::bulk_transfer ( NITRO_DIR d, uint8 ep, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   //int transferred=0;
   if(ep == READ_EP) {
     if(m_read_ep) {
       ep = m_read_ep;
     }
   } else {
     if(m_write_ep) {
       ep = m_write_ep;
     }
   }

   usb_async_tx_struct tx_struct;
   tx_struct.dev = m_dev;
   tx_struct.ep = ep;
   tx_struct.length = length;
   tx_struct.data = data;
   tx_struct.queued = 0;
   tx_struct.transferred = 0;
   tx_struct.err=0;
   tx_struct.timeout=timeout;

   while (tx_struct.transfers.size() < NITRO_TX_QUEUE_DEPTH && tx_struct.queued < length ) {
       usb_tx_submit_helper(&tx_struct,NULL);
   }
 
   while (tx_struct.transferred < length && !tx_struct.err) {
       timeval tv;
       // TODO only wait the remining portion of the time... 
       // need to use gettimeofday to subtract delta of 
       // current ellapsed time.
       tv.tv_sec = timeout / 1e3;
       tv.tv_usec = (timeout % 1000) * 1e3;
       int ret;
       if (ret=libusb_handle_events_timeout(NULL, &tv )) {
          tx_struct.err=ret;
          break; 
       }
   }

   for (std::vector<libusb_transfer*>::iterator itr = tx_struct.transfers.begin();
        itr != tx_struct.transfers.end(); 
        ++itr ) {
        libusb_cancel_transfer(*itr);
   }

   while ( tx_struct.transfers.size() ) {
       usb_debug ( "handling usb completion events." );
       libusb_handle_events_completed(NULL,NULL);
   }

   if (tx_struct.err || tx_struct.transferred < length ) {
     throw Exception ( USB_COMM, "bulk transfer fail", tx_struct.err );       
   }

   return length;
}

void USBDevice::impl::close() {
    if (m_dev) {
      libusb_release_interface(m_dev,0);
      libusb_close(m_dev);
      m_dev=NULL;
	  usb_debug ( "Closed device." );
    }
}

std::wstring USBDevice::impl::get_device_serial(uint32 vid, uint32 pid, uint32 index ) {
    IndexOpener d ( index );
    iter_devices( vid, pid, d );
    if (!d.m_dev) throw Exception ( USB_PROTO, "Insufficient devices connected." );
    std::wstring serial;
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

} // end nitro namespace
