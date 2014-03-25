
#include <cstdio>

#include <iostream>
#include <nitro/types.h>


using namespace std;

#ifdef WIN32
#define UD_API __declspec(dllexport)
#else
#define UD_API
#endif

extern "C" {

UD_API void* ud_init ( const char* args[],void* ud ) {
    printf ( "I like to initialize\n" );
    return 0;
}
UD_API int ud_read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, size_t* transferred, uint32 timeout, void* ud ) {
    printf ( "I like to read things.\n" );
    *transferred = length;
    return 0;
}
UD_API int ud_write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, size_t* transferred, uint32 timeout, void* ud ) { 
    printf ( "I like to write things.\n" );
    *transferred = length;
    return 0;
}
UD_API void ud_close(void* ud) {
    printf ( "I like to close things.\n" );
}

}
