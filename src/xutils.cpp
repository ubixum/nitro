
#ifdef WIN32
#include <Windows.h>
#include <cstdlib> // path operations on Windows
#define REG_INST_DIR L"Software\\Ubixum\\Nitro\\InstDir"
#else
#include <libgen.h> // dirname/basename on Linux
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#endif

#include <cstring>
#include <cstdlib>

#include <nitro/error.h>

#include "xutils.h"

#ifdef DEBUG_UTILS
#include <iostream>
#define utils_debug(x) std::cout << x << std::endl;
#else
#define utils_debug(x)
#endif

using namespace Nitro;

std::string xdirname ( const std::string &path ) {

#ifdef WIN32

	char path_buffer[_MAX_PATH];
	if ( _fullpath ( path_buffer, path.c_str(), _MAX_PATH ) == NULL) throw Exception ( PATH_LOOKUP, "Unable to lookup absolute path", path );
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath ( path_buffer, drive, dir, NULL, NULL );
	char res[_MAX_DIR+_MAX_PATH];
	sprintf ( res, "%s%s", drive, dir );
	return std::string(res);

#else
    char* path_tmp = strdup ( path.c_str() );
    char* dir = ::dirname(path_tmp);
    std::string ret(dir);
    free(path_tmp);
    return ret; 
#endif

}

std::string xjoin ( const std::string &path, const std::string &file ) {
    #ifdef WIN32 
    const static std::string sep = "\\";
    #else
    const static std::string sep = "/";
    #endif

    if ( file.length()>0 && file.substr(0,1) == sep ) {
        return file;
    }

    if ( path.length() < 1 ) return file;

    if ( path.substr ( path.length()-1, 1 ) != sep ) return path + sep + file;
    return path + file;

}


#ifdef WIN32
std::string get_inst_dir() {
	std::string inst_dir;
   	HKEY reg_handle;
	if ( RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
			REG_INST_DIR,
			0, KEY_READ,
			&reg_handle
		) == ERROR_SUCCESS ) {

		DWORD size; LONG ret;
		if ( (ret = RegQueryValueEx (
			reg_handle, NULL, NULL, NULL,
			NULL, &size
			)) == ERROR_SUCCESS ) {

            char *buffer = new char[size];
			if ( RegQueryValueEx ( reg_handle, NULL, NULL, NULL, (LPBYTE)buffer, &size ) == ERROR_SUCCESS ) {
				try {
                    std::wstring wbuf ((wchar_t*)buffer );
                    inst_dir.assign ( wbuf.begin(), wbuf.end() );
                    utils_debug ( "Retrieved Buffer: " << inst_dir );
                } catch ( const Exception &e ) {
                    utils_debug ( e );
                } // silent ignore again
			} else {
				utils_debug ( "Didn't query value." );
			}
			delete [] buffer;
		} else {
			utils_debug ( "Didn't query value size: " << ret );
		}

		RegCloseKey(reg_handle);
	
	} else {
		// else silently ignore error.  Pehraps PYTHONPATH will work.
		utils_debug ( "Couldn't open hkey" );
	}
    return inst_dir;
 
}
#endif


#ifdef WIN32
#define CWD_MAX MAX_PATH
#else
#define CWD_MAX PATH_MAX
#endif

std::string xgetcwd() {

   
#ifdef WIN32
   TCHAR path[CWD_MAX+1];
   DWORD ret = GetCurrentDirectory(CWD_MAX+1, path);

   if( ret == 0 ) throw Exception ( PATH_LOOKUP, "Error retrieving current directory." , (int)ret );
   
   if(ret > CWD_MAX) throw Exception ( PATH_LOOKUP, "Error retrieving current directory." , (int)ret );

   std::wstring ws ( path );
   std::string s;
   s.assign ( ws.begin(), ws.end() );
   return s;
  
#else    
	char path[CWD_MAX+1];
    char *ret = getcwd ( path, CWD_MAX+1 );
    if (!ret) throw Exception ( PATH_LOOKUP, "Error retrieving current directory." , errno );
	return std::string(path);
#endif

	

}
