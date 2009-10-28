

#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <nitro.h>

using namespace Nitro;

class ErrorTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( ErrorTest );
    CPPUNIT_TEST ( testRef );
    CPPUNIT_TEST ( testCopy );
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {
        }
        void throwFunc() {
            throw Exception ( 1, "An Exception Occurred." , "I like user data");
        }
        void testRef () {
            try {
                throwFunc();
                CPPUNIT_FAIL( "No Exception!" );
            } catch ( Exception &e) {
                CPPUNIT_ASSERT_EQUAL ( Nitro::STR_DATA, e.userdata().get_type() );
                CPPUNIT_ASSERT_EQUAL ( 1, e.code() );
                CPPUNIT_ASSERT_EQUAL ( std::string ("I like user data"), (std::string) e.userdata() );
            }
        }
        void testCopy() {
            try {
                throwFunc();
                CPPUNIT_FAIL( "No Exception!" );
            } catch ( Exception e ) {
                CPPUNIT_ASSERT_EQUAL( Nitro::STR_DATA, e.userdata().get_type() );
                CPPUNIT_ASSERT_EQUAL( 1, e.code() );
            }
        }

};


CPPUNIT_TEST_SUITE_REGISTRATION ( ErrorTest );
