

#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>
#include <memory>

using namespace Nitro;

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
                        
            std::auto_ptr<UserDevice> ud(new UserDevice(USERDLL_PATH, NULL, NULL));
            ud->get ( 0, 0 );
            ud->set ( 1, 2, 3 );
            ud->close();
            
        }
};


CPPUNIT_TEST_SUITE_REGISTRATION ( UserDeviceTest );
