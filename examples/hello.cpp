
#include <nitro.h>

#define BUFFER_LEN 100

using namespace Nitro;

int main() {
  int timeout = 1000;      // one second
  uint16 value=0;
  uint8 buffer[BUFFER_LEN];
  USBDevice dev(0x1fe1,0x7848);
  // open 1st device
  dev.open();

  // get a sample register value
  int i = dev.get( 0, 1, timeout);

  // set another register to whatever the 1st one returned
  dev.set(0, 2, i, timeout);

  // read some data into a buffer
  dev.read(0, 3, buffer, BUFFER_LEN, timeout);

  // process the buffer...

  // close device
  dev.close();

  return 0;
}
