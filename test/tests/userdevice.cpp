

#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>
#include <memory>
#include <iostream>

using namespace Nitro;
using namespace std;

#ifdef WIN32
#define USERDLL_PATH "userdev_dll.dll"
#else
#define USERDLL_PATH "./userdevice.so"
#endif


class UserDeviceTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( UserDeviceTest );
    CPPUNIT_TEST ( testUD );
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {
        }

        void testUD () {
                        
            try {
                std::auto_ptr<UserDevice> ud(new UserDevice(USERDLL_PATH, NULL, NULL));
                ud->get ( 0, 0 );
                ud->set ( 1, 2, 3 );
                ud->close();
            } catch ( Exception &e ) {
                cout << "User Device Fail: " << e << endl;
                CPPUNIT_FAIL ( "User device exception." );
            }
            
        }
};


CPPUNIT_TEST_SUITE_REGISTRATION ( UserDeviceTest );
