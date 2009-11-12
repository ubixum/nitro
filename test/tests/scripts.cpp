
#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>

#include "memorydevice.h"

using namespace Nitro;
using namespace std;

class ScriptTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( ScriptTest );
    CPPUNIT_TEST ( testScript );
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {

        }
        void testScript() {
            
            Scripts sc;
            CPPUNIT_ASSERT_NO_THROW( sc.import ( "s", "script_test.py" ) );
            vector<string> funcs = sc.func_list("s");
            CPPUNIT_ASSERT_EQUAL ( 8, (int) funcs.size() ); 

            CPPUNIT_ASSERT_THROW ( sc.get_params ( "s", "non-existent func" ), Exception );
            NodeRef args;
            CPPUNIT_ASSERT_NO_THROW ( args = sc.get_params( "s", "MyFunction" ) );
            DIAttrIter itr=args->attrs_begin();
            CPPUNIT_ASSERT ( itr != args->attrs_end() );
            CPPUNIT_ASSERT_EQUAL ( string ( "int" ), (string) itr->second );
            string pname = itr->first;
            args->set_attr ( pname, 5 );
            DataType result(0);
            CPPUNIT_ASSERT_NO_THROW ( result = sc.exec("s", "MyFunction", args ) );
            CPPUNIT_ASSERT_EQUAL ( 15, (int) result );

            // test float
            // func returns x/7.2
            args = Node::create("args");
            args->set_attr ( "f", 97.92 );
            double dres = sc.exec("s","TestFloat",args);
            CPPUNIT_ASSERT_EQUAL ( dres, 13.6 );

            // test device
            args = Node::create("args");
            // we'll re-use the userdevice 
            MemoryDevice dev;
            dev.set ( 0, 5, 0x1234 );
            args->set_attr ( "dev", dev );
            args->set_attr ( "term", 0 );
            args->set_attr ( "reg", 5 );
            CPPUNIT_ASSERT_NO_THROW ( result = sc.exec ("s", "DevFunction", args ) );
            CPPUNIT_ASSERT_EQUAL ( 0x1234, (int) result );


            // test exc
            try {
                args = Node::create("no args");
                sc.exec ( "s" , "broken_func", args );
                CPPUNIT_FAIL ( "Func didn't throw" );
            } catch ( Exception &e ) {
                CPPUNIT_ASSERT_EQUAL ( 5, e.code() ); 
                DataType userdata = e.userdata();
                CPPUNIT_ASSERT_EQUAL ( NODE_DATA, userdata.get_type() );
                NodeRef node = (NodeRef)userdata;
                CPPUNIT_ASSERT_EQUAL ( string( "hi" ), (string) node->get_attr("test1" ) );
            }


            // test buffer
            uint8 *buf = new uint8[4]; // = { 1, 2, 3, 4 };
            for (int i=0;i<4;++i) buf[i]=i+1;
            DataType b = DataType::as_datatype(buf,4);
            args = Node::create("args");
            args->set_attr("buf",b);
            CPPUNIT_ASSERT_NO_THROW ( sc.exec ( "s", "buffer_function", args ) );
            CPPUNIT_ASSERT_EQUAL ( buf[3], (uint8) 5 ); 
            delete [] buf;


            // buffer again
            uint8 buf2[2];
            DataType b2 = DataType::as_datatype(buf2,2);
            args = Node::create("args");
            args->set_attr("dev",dev);
            args->set_attr("buf",b2);
            try {
                sc.exec ( "s", "buffer_function2", args );
            } catch ( const Exception &e) {
                CPPUNIT_FAIL(e.str_error().c_str());
            }
            // remember memory dev set 0, 5 to 0x1234?
            uint16 u16 = *((uint16*)buf2);
            CPPUNIT_ASSERT_EQUAL ((uint16) 0x1234, u16 );


            // bigint test
            args = Node::create("args");
            args->set_attr("dev", dev );
            XmlReader reader ( "dev.xml" );
            reader.read ( dev.get_di() );
            CPPUNIT_ASSERT_NO_THROW ( sc.exec ( "s", "bigint_test", args ) );

            CPPUNIT_ASSERT_NO_THROW ( sc.exec ( "s", "get_subreg_test", args ) );


        }
};


CPPUNIT_TEST_SUITE_REGISTRATION ( ScriptTest );
