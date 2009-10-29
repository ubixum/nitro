#include <Python.h>


#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <getopt.h>

#include "nitro.h"

#include <python_nitro.h>

#define DEFAULT_VID 0x1fe1
#define DEFAULT_PID 0x1212


using namespace std;
using namespace Nitro;

int parseatoi( const char* const a ) {
	istringstream is (a);
	int v;
	is >> setbase(0) >> v;
	return v;
}

int main ( int argc, char* argv[] ) {

	int vid=DEFAULT_VID,pid=DEFAULT_PID;
	char* ihxfile=NULL, c;
	bool errflag=false, dev_reset=false, do_set=false, do_get=false, do_read=false, do_write=false, do_shell=false;
    uint16 value=0;
    DataType term_addr(0), reg_addr(0);
    char *term_orig=NULL, *reg_orig=NULL;
    unsigned int nBytes=0;
    unsigned int timeout=1000;
    char* rdwr_file=NULL;
    char* xml_file=NULL;

	while ( (c=getopt(argc, argv, "hV:P:R:t:a:gs:r:n:w:i:x:S")) != -1 ) {
		switch (c) {
			case 'h':
				errflag=true;
				break;
			case 'V':
				vid=parseatoi(optarg);
				break;
			case 'P':
				pid=parseatoi(optarg);
				break;
			case 'R':
				dev_reset=true;
                ihxfile=optarg;
				break;
            case 't':
                term_orig = optarg;
                break;
            case 'a':
                reg_orig = optarg;
                break;
            case 'g':
                do_get=true;
                break;
            case 's':
                do_set=true;
                value=parseatoi(optarg);
                break;
            case 'S':
                do_shell=true;
                break;
            case 'r':
               do_read=true;
               rdwr_file = optarg;
               break;
            case 'w':
                do_write=true;
                rdwr_file=optarg;
                break;
            case 'n':
                nBytes=parseatoi(optarg);
                break;
            case 'i':
                timeout=parseatoi(optarg);
                break;
            case 'x':
                xml_file=optarg;
                break;
			case '?':
				errflag=true;
				break;
		}
	}
	if (errflag) {
		printf ( "Usage: testdi [options]\n"
				"\tGeneric Options:\n"
				"\t\t-h This Message\n"
				"\t\t-V The Vendor Id [default 0x%04x]\n"
				"\t\t-P The Product Id [default 0x%04x]\n"
                "\t\t-S Execute shell environment after other commands complete.\n"
				"\tReset the Firmware (Ignores all other options)\n"
				"\t\t-R Reset The Firmware (requires path to ihx file)\n"
				"\tTerminal Operations\n"
				"\t\t-t terminal addr (default 0)\n"
				"\t\t-a register addr (default 0)\n"
				"\t\t-s <value> set\n"
				"\t\t-g get\n"
                "\t\t-r <filename> read data, save to filename (requires -n)\n"
                "\t\t-n <nBytes> number of bytes to read\n"
                "\t\t-w <filename> write file data (uses file size by default, override with -n\n"
                "\t\t-i <timeout> the timeout in milliseconds to wait for operations (default 1000).\n"
                "\t\t-x <xmlfile> read device interface from xmlfile.  If used, causes terminal and register address to be interpreted as names strings instead of integer addresses.\n"
                , DEFAULT_VID, DEFAULT_PID
				);
		return 1;
	}


    if (!USBDevice::get_device_count(vid,pid)) {
        cout << "No Connected Nitro USB Devices." <<endl;
        return -1;
    }

    USBDevice dev(vid,pid);

    try {

        dev.open(0,dev_reset); // open 1st device 

	if (dev_reset) {

		ifstream in;
		in.open(ihxfile, ios::binary);
		if (!in.good()) {
			printf ( "Open File didn't work\n" );
			return -1;
		}

		// read the file size	
		in.seekg(0,ios::end);
		int length=in.tellg();
		in.seekg(0,ios::beg);

		char* buf = new char[length];	
		in.read(buf,length);
		in.close();

		printf ( "Read File of Length: %d\n" , length );
		// use set instead

		dev.load_firmware( buf, length );

		delete [] buf;

		printf ( "Device firmware reset.\n" );
   
	} else {

            if ( xml_file ) {
                XmlReader reader(xml_file); 
                NodeRef di = dev.get_di();
                reader.read(di);
                if ( term_orig ) {
                    term_addr = string(term_orig);
                }
                if ( reg_orig ) {
                    reg_addr = string(reg_orig);
                }
            } else {
                if (term_orig ) term_addr = parseatoi(term_orig);
                if (reg_orig ) reg_addr = parseatoi(reg_orig);
            }

    	    if (do_get) {
               cout << dev.get(term_addr,reg_addr,timeout) << endl;
           } else if (do_set) {
               dev.set(term_addr,reg_addr,value,timeout);
           } else if (do_read) {
               if (0==nBytes) {
                 printf ( "Specify number of bytes to read with -n\n" );             
               } else {
                 uint8 *read_buf=new uint8[nBytes];
                 dev.read ( term_addr, reg_addr, read_buf, nBytes, timeout );

                 printf ("save bytes to %s\n", rdwr_file );
                 ofstream out;
                 out.open( rdwr_file, ios::binary );
                 out.write ( (char*)read_buf, nBytes );
                 out.close();

                delete [] read_buf; // mem leak if throw
               }
           } else if (do_write) {
             ifstream in;
             in.open(rdwr_file, ios::binary);
             if (in.good()) {
                int bytes_to_write=nBytes;
                if (0==bytes_to_write) {
                    // in this case, default to file size.
                    in.seekg(0,ios::end);
                    bytes_to_write=in.tellg();
                    in.seekg(0,ios::beg);                
                }
                uint8 *write_buf = new uint8[bytes_to_write];
                in.read((char*)write_buf,bytes_to_write);

                dev.write(term_addr, reg_addr, write_buf, bytes_to_write, timeout);
                delete [] write_buf;
             } else {
                printf ( "Failed to open file '%s' for reading.\n", rdwr_file );
             }
           }
        }

    if (do_shell) {
        
		Scripts sc; // init in scripts set's up path information.		
		
        if (import_nitro()<0) {
            PyErr_Print();
            return -1;
        }
        PyObject* devobj = nitro_from_datatype( dev );
        if (!devobj) {
            PyErr_Print();
            return -1;
        }
        PyObject* globals = PyEval_GetBuiltins();
        if ( !globals ) {
            PyErr_Print();
            return -1;
        }
        PyDict_SetItemString ( globals, "dev", devobj );

        //Py_Main(1, argv);
		PyRun_InteractiveLoop ( stdin, "nitro" );

    }

    } // end try
    catch (Exception e) {
            cout << e << endl;
            return -1;
    }
	
    dev.close(); // this happens anyway when program exits.

	return 0;

}
