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

#include <nitro/userdevice.h>
#include <nitro/error.h>

#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HINSTANCE dll_handle;

#else

#include <dlfcn.h>
typedef void* dll_handle;

#endif

namespace Nitro {


typedef void* (*ud_init_func)(const char* [],void*);
typedef int (*ud_get_func)(uint32, uint32, uint32*, uint32, void*);
typedef int (*ud_set_func)(uint32, uint32, uint32, uint32,void*);
typedef int (*ud_rdwr_func)(uint32, uint32, uint8*, size_t, size_t*, uint32,void*);
typedef void (*ud_close_func)(void*);


struct UserDevice::impl {
    dll_handle m_handle;
    void* m_ud_userdat;
    ud_init_func ud_init;
    ud_get_func ud_get;
    ud_set_func ud_set;
    ud_rdwr_func ud_read;
    ud_rdwr_func ud_write;
    ud_close_func ud_close;

    impl(const std::string& path, const char* args[],void*);
    ~impl() throw();

};

#ifdef WIN32
/* Windows and Linux have completely different library handling routines */

#define SET_ERR_STR(s) {\
		LPVOID msg_buf;\
		FormatMessage (\
			FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,\
			NULL,\
			GetLastError(),\
			NULL,\
			(LPTSTR)&msg_buf,\
			0,\
			NULL );\
		std::wstring ws = (LPCTSTR)msg_buf;\
		s.assign(ws.begin(),ws.end());\
		LocalFree(msg_buf);\
	}


#define CHECK_SYM_ERR(func) if (!func) {\
		FreeLibrary(m_handle);\
		std::string s; \
		SET_ERR_STR(s); \
		throw Exception ( UD_NO_FUNC, s );\
	}


UserDevice::impl::impl(const std::string& path, const char* args[], void* ud)  {
    
	std::wstring wpath;
	wpath.assign(path.begin(), path.end());
	m_handle = LoadLibrary( wpath.c_str() );
    if (!m_handle) {
		std::string s;		
		SET_ERR_STR(s);
        throw Exception( UD_NOT_FOUND, s );
    }
   
     
    ud_init = (ud_init_func)GetProcAddress ( m_handle, "ud_init" );	
    CHECK_SYM_ERR(ud_init);
    ud_get = (ud_get_func)GetProcAddress ( m_handle, "ud_get" );
    CHECK_SYM_ERR(ud_get);
    ud_set = (ud_set_func)GetProcAddress ( m_handle, "ud_set" );
    CHECK_SYM_ERR(ud_set);
    ud_read = (ud_rdwr_func)GetProcAddress ( m_handle, "ud_read" );
    CHECK_SYM_ERR(ud_read);
    ud_write = (ud_rdwr_func)GetProcAddress ( m_handle, "ud_write" );
    CHECK_SYM_ERR(ud_write);
    ud_close = (ud_close_func)GetProcAddress ( m_handle, "ud_close" );
    CHECK_SYM_ERR("ud_close");

    m_ud_userdat = ud_init(args,ud);
}

UserDevice::impl::~impl() throw() {
    FreeLibrary(m_handle);
}

#else
/* The Linux Implementation */

#define CHECK_SYM_ERR(func) if ((error = dlerror()) != NULL) {\
    dlclose(m_handle);\
    throw Exception(UD_NO_FUNC,std::string("Unable to bind function address for " func ": " ) + error ); \
    }

UserDevice::impl::impl(const std::string& path, const char* args[],void* ud)  {
    char *error;

    m_handle = dlopen( path.c_str(), RTLD_NOW);
    if (!m_handle) {
        throw Exception( UD_NOT_FOUND, dlerror() );
    }

    // clear any error
    dlerror();
     
    *(void**)(&ud_init) = dlsym ( m_handle, "ud_init" );
    CHECK_SYM_ERR("ud_init");
    *(void**)(&ud_get) = dlsym ( m_handle, "ud_get" );
    CHECK_SYM_ERR("ud_get");
    *(void**)(&ud_set) = dlsym ( m_handle, "ud_set" );
    CHECK_SYM_ERR("ud_set");
    *(void**)(&ud_read) = dlsym ( m_handle, "ud_read" );
    CHECK_SYM_ERR("ud_read");
    *(void**)(&ud_write) = dlsym ( m_handle, "ud_write" );
    CHECK_SYM_ERR("ud_write");
    *(void**)(&ud_close) = dlsym ( m_handle, "ud_close" );
    CHECK_SYM_ERR("ud_close");

    m_ud_userdat = ud_init(args,ud);
}

UserDevice::impl::~impl() throw() {
    dlclose(m_handle);
}

#endif



UserDevice::UserDevice( const std::string &path, const char* args[],void* ud ) :  m_impl(new impl(path,args,ud)) {

}

UserDevice::~UserDevice() throw() {
    delete m_impl;
}

DataType UserDevice::_get( uint32 terminal_addr, uint32 reg_addr, uint32 timeout ) {
    uint32 val;
    int ret=m_impl->ud_get(terminal_addr, reg_addr, &val, timeout, m_impl->m_ud_userdat );
    if (ret) throw Exception ( ret, "UserDevice Get Error." );
    return val; 
}

void UserDevice::_set( uint32 terminal_addr, uint32 reg_addr, const DataType& type, uint32 timeout ) {
    int ret=m_impl->ud_set ( terminal_addr, reg_addr, (uint32)type, timeout, m_impl->m_ud_userdat ); 
    if (ret) throw Exception ( ret, "UserDevice Set Error." );
}

void UserDevice::_read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {
    size_t transferred;
    int ret=m_impl->ud_read ( terminal_addr, reg_addr, data, length, &transferred, timeout, m_impl->m_ud_userdat );
    if (ret) throw Exception ( ret, "UserDevice Read Error." );
    if (transferred != length ) throw Exception ( ret, "UserDevice didn't return sufficient data." );

}

void UserDevice::_write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {
    size_t transferred;
    int ret=m_impl->ud_write ( terminal_addr, reg_addr, const_cast<uint8*>(data), length, &transferred, timeout, m_impl->m_ud_userdat );
    if (ret) throw Exception ( ret, "UserDevice Write Error." );
    if (transferred != length ) throw Exception ( UD_PROTO, "UserDevice didn't write sufficient data." );
}

void UserDevice::_close() {
    m_impl->ud_close(m_impl->m_ud_userdat);
}



} // end nitro namespace
