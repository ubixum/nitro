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

#include <usb.h>

#include <cstdio>

typedef std::vector<struct usb_device*> DeviceList;
typedef std::vector<struct usb_device*>::iterator DeviceListItr;

#ifdef WIN32
/* On windows, libusb0 doesn't work with a timeout of 0 to mean no timeout.
   Instead, it seems to cause weird behavior.
*/
#define MAKETIMEOUT(t) t==0?0x7fffffff : t
#else
#define MAKETIMEOUT(t) t
#endif

struct USBDevice::impl : public usbdev_impl_core {
    /**
     * Need to initialize once per process (lib load)
     **/

    private:
        static bool initialized;
        uint32 m_vid;
        uint32 m_pid;
        uint16 m_ver; // firmware version
        usb_dev_handle* m_dev;
        static void check_init(); 
        void check_open() const;
        void config_device(struct usb_device* dev, bool override_version=false);
        static std::string device_serial ( struct usb_device* dev);
    public:

        impl(uint32 vid, uint32 pid): m_vid(vid),m_pid(pid),m_dev(NULL){}
        void open ( uint32 index, bool override_version );
        void open ( const std::string& serial );
        uint16 firmware_version();
		bool is_open() { return m_dev != NULL; }
        static void get_device_list(uint32 vid, uint32 pid, DeviceList& list) ;
        static uint32 get_device_count ( uint32 vid, uint32 pid );
        static std::string get_device_serial(uint32 vid, uint32 pid, uint32 index );
        static uint16 get_device_address(uint32,uint32,uint32);
   
        int control_transfer ( NITRO_DIR, NITRO_VC, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout );
        int bulk_transfer ( NITRO_DIR, uint8 ep, uint8* data, size_t length, uint32 timeout ); 

    
        void close();

};


bool USBDevice::impl::initialized = false;
void USBDevice::impl::check_init() {
    if (!initialized) {
        assert ( sizeof(uint8) == 1 );
        assert ( sizeof(uint16) == 2 );
        assert ( sizeof(uint32) == 4);
		assert ( sizeof(uint64) == 8);
    
    	usb_init();
        initialized=true;
		usb_debug ( "Initialized Lib USB0" );
    }
}

uint32 USBDevice::impl::get_device_count(uint32 vid, uint32 pid) {

  DeviceList list;
  impl::get_device_list(vid,pid,list);
  return list.size();

}

void USBDevice::impl::open(uint32 index, bool override_version) {


  DeviceList devlist;
  get_device_list(m_vid,m_pid,devlist);
  usb_debug ( "Open " << index << " of " << devlist.size() << " devices" ); 
  // NOTE additional !devlist.size check because devlist.size()-1 if size==0 is MAX_INT! 
  if (!devlist.size() || index > devlist.size()-1) {
    throw Exception(USB_COMM, "Insufficient number of connected devices attached.");
  }

  config_device(devlist.at(index), override_version);
}

void USBDevice::impl::open(const std::string& serial) {
   DeviceList list;
   get_device_list(m_vid,m_pid,list);

   for (DeviceListItr itr=list.begin(); itr != list.end(); ++itr ) {
         std::string test_serial = impl::device_serial(*itr);
         if ( serial == impl::device_serial ( *itr ) ) {
            config_device(*itr);
            return;
         }
   }

   throw Exception ( USB_COMM, "No connected device with serial found." );
}

void USBDevice::impl::check_open() const {
    if (!m_dev) throw Exception( USB_COMM, "IO method called on unopened device.");
}

void USBDevice::impl::get_device_list(uint32 vid, uint32 pid, DeviceList& devlist) {

  check_init();

  struct usb_bus *bus;
  struct usb_device *dev;

  // these calls will insure finding the device if 
  // the number of connected devices has changed
  // since the last time the call was made.
  usb_find_busses();
  usb_find_devices();

  for(bus = usb_get_busses(); bus; bus = bus->next) 
   {
     for(dev = bus->devices; dev; dev = dev->next) 
       {
         if(dev->descriptor.idVendor == vid
            && dev->descriptor.idProduct == pid) {
                devlist.push_back(dev);
           }
       }
   }
}

void USBDevice::impl::config_device(struct usb_device* dev, bool override_version) {

  m_ver = dev->descriptor.bcdDevice;
  usb_debug ( "Product firmware version: " << m_ver );

  if (!override_version) {
    if ( FIRMWARE_MAJOR < (m_ver >> 8) || FIRMWARE_MAJOR_MIN > (m_ver >> 8) ) {
      throw Exception ( USB_PROTO, "Current drivers do not support device firmware version.", m_ver );
    }
  }

  m_dev = usb_open(dev); 

  if (!m_dev) {
    throw Exception( USB_PROTO , "Failed to Open USB Device" , usb_strerror());
  }

  if (usb_set_configuration(m_dev,1) < 0) {
    usb_close(m_dev);
    m_dev=NULL;
    throw Exception( USB_PROTO, std::string("Failed to set device configuration: ") + usb_strerror());
  }
	  
  // claim interface
  if (usb_claim_interface(m_dev,0) < 0) {
	  usb_close(m_dev);
      m_dev=NULL;
      throw Exception( USB_PROTO, std::string("Failed to claim device interface: ") + usb_strerror());
  }

  if (usb_set_altinterface(m_dev,1) < 0 && usb_set_altinterface(m_dev,0)<0 ) {
	  usb_close(m_dev);
	  m_dev=NULL;
	  throw Exception( USB_PROTO, std::string("Failed to set interface alt setting: ")+ usb_strerror());
  }

  usb_debug ( "Device configured." );

}

uint16 USBDevice::impl::firmware_version () { 
    check_open();
    return m_ver;
}

int USBDevice::impl::control_transfer ( NITRO_DIR d, NITRO_VC c, uint16 value, uint16 index, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   int type = d == NITRO_OUT ? 0x40 : 0xc0;
   int ret=usb_control_msg ( m_dev, type, c, value, index, reinterpret_cast<char*>(data), length, MAKETIMEOUT(timeout) );
   usb_debug ( "control transfer length: " << length << " ret: " << ret << " " << (ret <0 ? usb_strerror() : "" ) );
   return ret;
}
int USBDevice::impl::bulk_transfer ( NITRO_DIR d, uint8 ep, uint8* data, size_t length, uint32 timeout ) {
   check_open();
   int ret;
   switch (d) {
    case NITRO_OUT:
       ret=usb_bulk_write ( m_dev, ep, reinterpret_cast<char*>(data), length, MAKETIMEOUT(timeout) );
       break;
    case NITRO_IN:
       ret=usb_bulk_read ( m_dev, ep, reinterpret_cast<char*>(data), length, MAKETIMEOUT(timeout) );
       break;
    default:
       throw Exception ( USB_COMM, "Invalid endpoint direction." );
   }

   usb_debug ( "lib usb ret: " << ret << ", " << (ret<0? usb_strerror() : "") );
   return ret;
}

void USBDevice::impl::close() {
    if (m_dev) {
      usb_release_interface(m_dev,0);
      usb_close(m_dev);
      m_dev=NULL;
	  usb_debug ( "Closed device." );
    }
}

class DevCloser {
    private:
        struct usb_dev_handle* m_dev;
    public:
        DevCloser(struct usb_dev_handle* dev) : m_dev(dev) {}
        ~DevCloser() { usb_close(m_dev); }
};

std::string USBDevice::impl::device_serial ( struct usb_device* dev) {

    struct usb_dev_handle* usbdev = usb_open(dev);
    if (!usbdev) {
        throw Exception ( USB_PROTO, "Error opening device." );
    }

    DevCloser closer(usbdev); // close at exit of function

    int ret=0;
    if (!dev->descriptor.iSerialNumber) {
        return "";
    } else {
        char buffer[20]; 
        ret = usb_get_string_simple( usbdev, dev->descriptor.iSerialNumber , buffer, 20); 
        if (ret<0) {
            throw Exception ( USB_PROTO, "Failed to query device serial number." );
        }
        return std::string(buffer, ret); 
    }
}



uint16 USBDevice::impl::get_device_address(uint32 vid,uint32 pid, uint32 index) {
   DeviceList list;
   get_device_list(vid,pid,list);
   if (!list.size() || list.size()-1 < index) {
     throw Exception ( USB_COMM, "Invalid index.  Not enough connected devices." );
   }
   struct usb_device *dev = list.at(index);
   uint32 bn;
#ifdef WIN32
#define sscanf sscanf_s
#define BUS_SCAN_FORMAT "bus-%u"
#else
#define BUS_SCAN_FORMAT "%03u"
#endif
   sscanf ( dev->bus->dirname, BUS_SCAN_FORMAT, &bn );
   usb_debug ( "bus location " << bn << " dirname " << dev->bus->dirname );
   uint8 dn = dev->devnum;
   return (uint16(bn) << 8) | dn;
   
}

std::string USBDevice::impl::get_device_serial(uint32 vid, uint32 pid, uint32 index ) {

    DeviceList list;
    get_device_list(vid,pid,list);
    if ( !list.size() || list.size()-1 < index ) {
        throw Exception( USB_COMM, "Invalid index.  Not enough connected devices." );
    }

    return device_serial( list.at(index) );

}
