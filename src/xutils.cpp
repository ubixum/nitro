
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
    string ret;
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
					std::string tmp;
					ret.assign ( wbuf.begin(), wbuf.end() );
					py_debug ( "Retrieved Buffer: " << tmp );
				} catch ( const Exception &e ) {
					py_debug ( e );
				} // silent ignore again
			} else {
				py_debug ( "Didn't query value." );
			}
			delete [] buffer;
		} else {
			py_debug ( "Didn't query value size: " << ret );
		}

		RegCloseKey(reg_handle);
	
	} else {
		// else silently ignore error.  Pehraps PYTHONPATH will work.
		py_debug ( "Couldn't open hkey" );
	}
    return ret;
 
}
#endif


std::string xgetcwd() {
    char path[PATH_MAX+1];
    char *ret = getcwd ( path, PATH_MAX+1 );
    if (ret) return std::string(ret);
    throw Exception ( PATH_LOOKUP, "Error retrieving current directory." , errno );
}
