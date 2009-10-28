// Copyright (C) 2009 Ubixum, Inc. 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#ifndef NITRO_USERDEVICE_H
#define NITRO_USERDEVICE_H

#include "device.h"

namespace Nitro {

/**
 * \ingroup devimpl
 * 
 * \brief Nitrogen User %Device
 *
 * The User %Device provides a way to implement your own device without exending Nitro::Device.  
 * This provides an easy way to use your device in different languages platforms without providing
 * custom bindings for each language/platform.
 *
 * To use UserDevice, you must provide a shared object or dynamic library.  The following functions 
 * must be exported.  You may use the following as a template for your shared library.
 *
 * \code
 * #include <nitro/types.h> // uint32 etc
 * #ifdef WIN32 
 * #define UD_API __declspec(dllexport)
 * #else
 * #define UD_API
 * #endif
 * extern "C" {
 *  // initialize your library, return a pointer to any user data you like.
 *  // user data will be passed to all other methods with this UserData instance.
 *  // The UserDevice class passes it's constructor variables without modification to this method.
 *  UD_API void* ud_init ( const char* args[], void* ); 
 *  UD_API int ud_get ( uint32 terminal_addr, uint32 reg_addr, uint32* value, uint32 timeout, void* ud );
 *  UD_API int ud_set ( uint32 terminal_addr, uint32 reg_addr, uint32 value, uint32 timeout, void* ud ) ;
 *  UD_API int ud_read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, size_t* transferred, uint32 timeout, void* ud );
 *  UD_API ud_write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, size_t* transferred uint32 timeout, void* ud ) ;
 *  UD_API void ud_close(void* ud);
 * }
 * \endcode
 *
 * UserDevice loads the shared object or DLL when created and then calls the DLL methods as requested by 
 * the client. If UserDevice failes to set the function addresses appropriately, it will
 * throw Nitro::Exception on creation.
 *
 **/
class DLL_API UserDevice : public Device {
private:
   struct impl;
   impl* m_impl;
protected:
   DataType _get( uint32 terminal_addr, uint32 reg_addr, uint32 timeout );
   void _set( uint32 terminal_addr, uint32 reg_addr, const DataType& type, uint32 timeout ) ;
   void _read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout );
   void _write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) ;
   void _close();
public:
    // core methods
    /**
     * \brief Construct a UserDevice
     * \param path Path to the shared object or DLL file.
     * \param args Array of NULL-terminated strings for the DLL init method.
	 * \param ud Any application specific data you wish to pass to your DLL init method.
     **/
    UserDevice(const std::string &path, const char* args[], void* ud);
    ~UserDevice() throw();

        
};

} 
#endif
