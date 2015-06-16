

#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>

#include "memorydevice.h"

using namespace Nitro;
using namespace std;

class DeviceTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( DeviceTest );
    CPPUNIT_TEST ( testGetSet );
    CPPUNIT_TEST ( testInt );
    CPPUNIT_TEST ( testNarrow);
    CPPUNIT_TEST ( testValuemap );
    CPPUNIT_TEST ( testDictSet );
    CPPUNIT_TEST ( testBigReg );
    CPPUNIT_TEST ( testArray );
    CPPUNIT_TEST ( testMutex );
    CPPUNIT_TEST ( testVerify );
    CPPUNIT_TEST ( testBuffers );
    CPPUNIT_TEST_SUITE_END();

    MemoryDevice dev;

    public:
        void setUp() {
            XmlReader reader ("dev.xml");
            reader.read(dev.get_di());
            //dev.enable_mode(Device::LOG_IO);
        }
        void tearDown() {

        }
        void testGetSet() {
            try {
                CPPUNIT_ASSERT_NO_THROW( dev.set ( 0, 1 , 21 ) );
                CPPUNIT_ASSERT ( dev.get ( 0, 1 ) == 21 );

                // reg 1 is 18 bits wide
                CPPUNIT_ASSERT_NO_THROW ( dev.get ( 0, string("reg1"), 0 ) );
                CPPUNIT_ASSERT_NO_THROW ( dev.get ( string("Terminal1"), string("reg1"), 0 ) );
                CPPUNIT_ASSERT_NO_THROW ( dev.get ( string("Terminal1"), 0 , 0 ) );
                CPPUNIT_ASSERT_NO_THROW ( dev.get ( string("Terminal1"), string("reg1"), 0 ) );

                // should be 
                // 1    0
                // 1010100000 = 672
                //cout << " -- get -- 672 -- " << endl;
                CPPUNIT_ASSERT ( dev.get ( 0, "reg1" ) == 672 ); //string("reg1") ) == 672 );

                // should set a few bits in each register
                // 00100 00011 00010 0001
                dev.set ( 0, string("reg1"), 0x20c41, 0 );
                CPPUNIT_ASSERT_EQUAL ( 1, (int32)dev.get ( 0, 0 ) );
                CPPUNIT_ASSERT_EQUAL ( 2, (int32)dev.get ( 0, 1 ) );
                CPPUNIT_ASSERT_EQUAL ( 3, (int32)dev.get ( 0, 2 ) );
                CPPUNIT_ASSERT_EQUAL ( 4, (int32)dev.get ( 0, 3 ) );

                // 6 bits 101010 (42)
                CPPUNIT_ASSERT_NO_THROW ( dev.set ( 0, "reg2", 0x2A ) );
                  
                 
                CPPUNIT_ASSERT_EQUAL(0x02, (int32)dev.get ( 0, "reg2.sub1" ) ) ;
                CPPUNIT_ASSERT_EQUAL(0x05, (int32)dev.get ( 0, "reg2.sub2" ) ) ;
                // 111010
                dev.set ( 0, "reg2.sub2", 0x07 ) ;
                CPPUNIT_ASSERT_EQUAL(0x3a , (int32)dev.get  ( 0, "reg2" ) ) ;
                CPPUNIT_ASSERT_EQUAL(0x07, (int32)dev.get ( 0, "reg2.sub2" ) );
            } catch ( Exception &e) {
                //cout << "Unexpected exception in get/set test: " << e << endl;
                CPPUNIT_FAIL ( "Unexpected exception." );
            }
        }

    void testInt() {
       uint32 bytes = 0x4030201;
       dev.write ( "int_term", "int", (uint8*)&bytes, 4 );
       uint32 res = dev.get ( "int_term", "int" );
       CPPUNIT_ASSERT_EQUAL ( bytes, res );

       CPPUNIT_ASSERT_EQUAL ( 0x201, (int32) dev.get ( "int_term" , "int.lw" ) );
       CPPUNIT_ASSERT_EQUAL ( 0x403, (int32) dev.get ( "int_term", "int.hw" ) );

    }

    void testNarrow() {
       try {
           dev.set ( "narrow_term", 0, 0x12 );
           dev.set ( "narrow_term", 1, 0x1234 ); // should case to 0x34 since too big for 8 bit addr 

           uint32 narrow_addr = dev.get_di()->get_child("narrow_term")->get_attr("addr");

           CPPUNIT_ASSERT_EQUAL ( 0x12, (int32) dev.get ( "narrow_term", 0 ) );
           CPPUNIT_ASSERT_EQUAL ( 0x34, (int32) dev.get ( "narrow_term", 1 ) );

           // same thing should work if set by addr
           //
           dev.set( narrow_addr, 0, 0x56 );
           dev.set( narrow_addr, 1, 0x7890 );

           CPPUNIT_ASSERT_EQUAL ( 0x56, (int32) dev.get ( "narrow_term", 0 ) );
           CPPUNIT_ASSERT_EQUAL ( 0x90, (int32) dev.get ( "narrow_term", 1 ) );

           // non existent di should do two byte read/writes 

           dev.set ( 0x6666, 0, 0x12345 );
           CPPUNIT_ASSERT_EQUAL ( 0x2345, (int32) dev.get( 0x6666, 0 ) );

       } catch ( Exception &e ) {
          cout << "error " << e << endl;
       }

   }

    void testValuemap() {
        dev.set ( "Terminal1", "reg1", "evens" );
        CPPUNIT_ASSERT_EQUAL ( 0x2aaaa , (int) dev.get ( "Terminal1", "reg1" ) );
        dev.set ( "Terminal1", "reg1", "odds" );
        CPPUNIT_ASSERT_EQUAL ( 0x15555, (int) dev.get ("Terminal1", "reg1" ) );

        dev.set ( "Terminal1", "reg2" , 0  );
        dev.set ( "Terminal1", "reg2.sub1", 7 );
        dev.set ( "Terminal1", "reg2.sub2", "7" );
        CPPUNIT_ASSERT_EQUAL ( 63 , (int) dev.get ( "Terminal1", "reg2" ) );

    }

    void testDictSet() {
        NodeRef vals = Node::create("vals");
        // 110010
        vals->set_attr("sub1", 2 );
        vals->set_attr("sub2", "6" ); // valuemap value
        dev.set ( "Terminal1", "reg2", vals );
        CPPUNIT_ASSERT_EQUAL ( 50, (int) dev.get( "Terminal1", "reg2" ) );
        // 110011
        vals = Node::create("newvals" );
        vals->set_attr("sub1", 3 );
        dev.set ( "Terminal1", "reg2" , vals );
        CPPUNIT_ASSERT_EQUAL ( 51, (int) dev.get( "Terminal1", "reg2" ) ); 

        // 000011
        vals->set_attr("sub2", 0);
        dev.set ( "Terminal1", "reg2", vals );
        CPPUNIT_ASSERT_EQUAL ( 3, (int) dev.get("Terminal1", "reg2" ) );


        CPPUNIT_ASSERT_NO_THROW ( vals = dev.get_subregs ( "Terminal1", "reg2" ) );
        CPPUNIT_ASSERT ( vals->has_attr("sub1" ) );
        CPPUNIT_ASSERT ( vals->has_attr("sub2" ) );
        CPPUNIT_ASSERT_EQUAL ( 3, (int) vals->get_attr("sub1") );
        CPPUNIT_ASSERT_EQUAL ( 0, (int) vals->get_attr("sub2") );


    }


    void testBigReg() {
        //cout << " -- set big_sub2 -- " << endl;
        dev.set ( "int_term" , "wide_reg.big_sub2", 0xd );
        DataType dt = dev.get ( "int_term", "wide_reg.big_sub2" );
        CPPUNIT_ASSERT_EQUAL ( 0xd, (int32) dt ); //dev.get ( "int_term", "wide_reg.big_sub2" ) );
        //cout << " -- set big_sub3 -- " << endl;
        dev.set ( "int_term", "wide_reg.big_sub3", (uint32)0x12345678 );
        CPPUNIT_ASSERT_EQUAL ( (uint32)0x12345678, (uint32) dev.get("int_term", "wide_reg.big_sub3" ) );
        //cout << " -- set big_sub2 -- " << endl;
        dev.set ( "int_term", "wide_reg.big_sub2", 1 );
        //dev.debug();
        CPPUNIT_ASSERT_EQUAL ( (uint32)0x12345678, (uint32) dev.get("int_term", "wide_reg.big_sub3" ) );
        
        // 37 bits, 4 bits, 33 bits = 74 bits
        vector<DataType> big1;
        big1.push_back ( (uint32)0x12345678 );
        big1.push_back ( (uint32)0x90123456 );
        big1.push_back ( (uint32)0x3ff );
        DataType bigdt = DataType::as_bigint_datatype(big1);
        CPPUNIT_ASSERT_NO_THROW ( dev.set ( "int_term", "wide_reg" , bigdt) );

        bigdt = dev.get( "int_term", "wide_reg" );
        CPPUNIT_ASSERT_EQUAL ( BIGINT_DATA, bigdt.get_type() );
        big1 = bigdt.as_bigints(bigdt);

        CPPUNIT_ASSERT_EQUAL ( (uint32) 0x12345678 , (uint32) big1.at(0) );
        CPPUNIT_ASSERT_EQUAL ( 2, (int) dev.get ( "int_term", "wide_reg.big_sub2" ) );

        bigdt = dev.get( "int_term", "wide_reg.big_sub1" );
        CPPUNIT_ASSERT_EQUAL ( BIGINT_DATA, bigdt.get_type() );
        big1 = bigdt.as_bigints(bigdt);
        CPPUNIT_ASSERT_EQUAL ( 2, (int) big1.size() ); // 37 bits
        CPPUNIT_ASSERT_EQUAL ( (uint32) 0x12345678, (uint32) big1.at(0) );
        CPPUNIT_ASSERT_EQUAL ( (uint32) 0x16, (uint32) big1.at(1) );


        CPPUNIT_ASSERT_EQUAL ( 184 , (int) dev.get_register ( "int_term", "many_subregs" )->get_attr("width" ) );

        dev.clear();
        dev.set("int_term", "many_subregs.fifteen" , 500 );
        dev.set("int_term", "many_subregs.fourteen", 0xfff );
        dev.set("int_term", "many_subregs.sixteen", 0xfff );
        CPPUNIT_ASSERT_EQUAL ( 500, (int) dev.get ( "int_term", "many_subregs.fifteen" ) );


      }

    void testMutex() {
        // needs expanded to test with threads too

        // test repeated calls to lock don't block single threads
        dev.lock();
        dev.lock();
        dev.unlock();
        dev.unlock();
    }

    void testArray() {
       CPPUNIT_ASSERT_NO_THROW ( dev.get ( "Terminal1" , "array_reg1" ) );
       CPPUNIT_ASSERT_NO_THROW ( dev.get ( "Terminal1" , "array_reg1[0]" ) ); 
       vector<DataType> data ;
       data.push_back(3); // array_reg1 is 20 fields 
       CPPUNIT_ASSERT_THROW ( dev.set ( "Terminal1", "array_reg1", data ), Exception ); // throw because we need 20 of them 
       for (int i=0;i<19;++i) data.push_back(i);
       CPPUNIT_ASSERT_NO_THROW ( dev.set ( "Terminal1", "array_reg1", data ) );
       CPPUNIT_ASSERT_NO_THROW ( dev.set ( "Terminal1", "array_reg1[5]" , 20 ) );
       CPPUNIT_ASSERT_THROW ( dev.set ( "Terminal1", "array_reg1[20]" , 5 ), Exception ); // array out of bounds 
       CPPUNIT_ASSERT_EQUAL ( 20 , (int) dev.get("Terminal1", "array_reg1[5]" ) );
       CPPUNIT_ASSERT_EQUAL ( 2, (int) dev.get ("Terminal1", "array_reg1[3]" ) );

       data.clear();
       data = dev.get ( "Terminal1", "array_reg1" );
       CPPUNIT_ASSERT_EQUAL ( 20, (int) data.size() );

       // array 2 4 sets of 7
       //
       data.clear();
       for (int i=0;i<4;++i) {
         data.push_back ( i << 4 ); // should fit in 7 bits
       }
       
       CPPUNIT_ASSERT_NO_THROW ( dev.set("Terminal1", "array_reg2" , data ) ); 
       CPPUNIT_ASSERT_EQUAL ( 0x30, (int) dev.get("Terminal1", "array_reg2[3]" ) );


       // array 3 3 sets of 18
    }

    void testVerify() {
        
        try {
            dev.clear();
            dev.set_modes(0); // clear any defaults
            dev.enable_mode(Device::GETSET_VERIFY);
            dev.set_error_mode(true);
            try {
              dev.set ( 0, 0, 1 ); // 1 set, 1 get.. one should throw
              CPPUNIT_FAIL ( "set didn't throw." );
            } catch ( const Exception &e ) {          
                //cout << e;
                CPPUNIT_ASSERT_EQUAL ( DEVICE_OP_ERROR, (Nitro::NITRO_ERROR)e.code() );
            }

            dev.enable_mode(Device::DOUBLEGET_VERIFY);
            dev.disable_mode(Device::GETSET_VERIFY);
            try {
                CPPUNIT_ASSERT_NO_THROW ( dev.set ( "Terminal1", "reg1", 0xab ) );
                CPPUNIT_ASSERT_EQUAL ( 0xab, (int) dev.get ( "Terminal1", "reg1" ) ); // two gets, one should throw
                CPPUNIT_FAIL ( "get didn't throw" );
            } catch ( const Exception &e ) {
                CPPUNIT_ASSERT_EQUAL ( DEVICE_OP_ERROR, (Nitro::NITRO_ERROR)e.code() );
            }


            dev.disable_mode(Device::GETSET_VERIFY);
            dev.disable_mode(Device::DOUBLEGET_VERIFY);
            dev.enable_mode( "int_term", Device::STATUS_VERIFY);
            try {

                CPPUNIT_ASSERT_NO_THROW ( dev.get( "Terminal1", 0 ) ); // only int_term should throw
                CPPUNIT_ASSERT_NO_THROW ( dev.get( "Terminal1", 0 ) );

                dev.get( "int_term", 0 ); // one of these should throw
                dev.get( "int_term", 0 );
                CPPUNIT_FAIL ( "status verify didn't throw" );
            } catch ( const Exception &e ) {
                CPPUNIT_ASSERT_EQUAL ( DEVICE_OP_ERROR, (Nitro::NITRO_ERROR)e.code() );
            }


            dev.enable_mode ( Device::DOUBLEGET_VERIFY );
            dev.enable_mode ( Device::GETSET_VERIFY );        
            dev.enable_mode ( Device::STATUS_VERIFY );
            CPPUNIT_ASSERT_EQUAL ( (uint32) (Device::DOUBLEGET_VERIFY|Device::GETSET_VERIFY|Device::STATUS_VERIFY), dev.get_modes() );

            dev.enable_mode ( 10, Device::GETSET_VERIFY );
            CPPUNIT_ASSERT_EQUAL ( (uint32) Device::GETSET_VERIFY , dev.get_modes ( 10 ) );


            dev.enable_mode( "int_term", Device::RETRY_ON_FAILURE );
            //cout << " -- retry failure -- " << endl;
            dev.set_error_step(3);
            try {
                dev.set ( "int_term" , "int", (uint32) 0xabcdef );
                CPPUNIT_ASSERT_EQUAL ( 0xcdef , (int) dev.get("int_term", "int.lw" ) ); 
                CPPUNIT_ASSERT_EQUAL ( 0xab , (int) dev.get("int_term", "int.hw" ) );
            } catch ( const Exception &e ) {
                //cout << "Fail retry test: " << e << endl;
                CPPUNIT_FAIL ( "modes didn't retry." );
            }
            // make sure it would have failed without the retry
            dev.disable_mode ("int_term", Device::RETRY_ON_FAILURE );
            CPPUNIT_ASSERT_THROW ( dev.set ( "int_term" , "int", (uint32) 0xabcdef ), Exception );


            try {
                dev.set_modes(0);
                dev.set_modes("int_term", 0);
                dev.enable_mode( "int_term", Device::CHECKSUM_VERIFY);
                dev.set_error_mode(false);
                dev.get("int_term", "int");
                dev.set("int_term", "int", 7 );
                uint8 buf[100];
                dev.read( "Terminal1", 0, buf, 100 );
            } catch ( Exception &e ) {
                CPPUNIT_FAIL ( "checksum problem" );
            }
        } catch ( Exception &e ) {
            cout << e << endl;
            CPPUNIT_FAIL ( "Unexpected Exception" );
        }

    }

    void testBuffers() {
        uint8 *buffer = new uint8[1024*1024];
        dev.read( "int_term", 0, buffer, 1024*1024 );
		delete [] buffer;

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION ( DeviceTest );

