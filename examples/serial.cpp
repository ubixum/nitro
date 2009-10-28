
#include <string>

#include <stdio.h>

#include <nitro.h>

const int vid=0x1fe1;
const int pid=0x7848;

using namespace std;
using namespace Nitro;

int main() {

    // test opening more than once device
    
    int c=0;
    c=USBDevice::get_device_count(vid,pid);

    if (c>0) {
        printf ( "%d attached nitro devices.\n" , c );
        int i;
        for (i=0;i<c;++i) {
            string serial = USBDevice::get_device_serial(vid,pid,i); 
            printf("Device %d serial %s\n", i, serial.c_str() );
            USBDevice dev(vid,pid);
            dev.open ( serial );
            dev.close();
        }
    } else {
        printf ( "No attached USB Devices.\n" );
    }
    
    return 0;

}
