

#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>

using namespace Nitro;

class TypeTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( TypeTest );
    CPPUNIT_TEST ( testConstruct );
    CPPUNIT_TEST ( testInt );
    CPPUNIT_TEST ( testUint );
    CPPUNIT_TEST ( testFloat );
    CPPUNIT_TEST ( testStr );
    CPPUNIT_TEST ( testNode );
    CPPUNIT_TEST ( testList );
    CPPUNIT_TEST ( testBigInt );
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {
        }
        void testConstruct() {
            DataType t(1);
            CPPUNIT_ASSERT_EQUAL ( t.get_type(), INT_DATA );
            DataType t2( (uint32)2 );
            CPPUNIT_ASSERT_EQUAL ( t2.get_type(), UINT_DATA );
            DataType t3("Hello");
            CPPUNIT_ASSERT_EQUAL ( t3.get_type(), STR_DATA );
        }
        void testInt() {
            DataType t = -1;
            CPPUNIT_ASSERT ( t == -1 ); 
            CPPUNIT_ASSERT ( -1 == (int32)t );
            CPPUNIT_ASSERT ( t == DataType(-1) );
            CPPUNIT_ASSERT ( t != -2 );
            t = 5;
            CPPUNIT_ASSERT_EQUAL ( 5 , (int32) t );            
            CPPUNIT_ASSERT_THROW ( (NodeRef)t, Exception );
        }
        void testUint() {
            DataType t(1);
            CPPUNIT_ASSERT ( t == 1 ); 
            CPPUNIT_ASSERT ( 1 == (int32)t );
            CPPUNIT_ASSERT ( t == DataType(1) );
        }

        void testFloat() {
            DataType d(1.0);
            CPPUNIT_ASSERT_EQUAL(FLOAT_DATA, d.get_type());

            CPPUNIT_ASSERT ( d == 1.0 );
            CPPUNIT_ASSERT_EQUAL ( (uint32) 1, (uint32) d );
            CPPUNIT_ASSERT_EQUAL ( (int32) 1, (int32) d );
            CPPUNIT_ASSERT_EQUAL ( (double) 1, (double) d );
            CPPUNIT_ASSERT ( d == DataType(1.0) );

            d = 16.732;
            CPPUNIT_ASSERT_EQUAL ( 16.732, (double) d );
        }

        void testStr() {
            DataType t("Hi");
            CPPUNIT_ASSERT ( t == "Hi" ); 
            CPPUNIT_ASSERT ( "Hi" == (std::string)t );
            CPPUNIT_ASSERT ( t == DataType("Hi") );
        }

        void testNode() {
            NodeRef n = Node::create("Some Node");
            DataType t ( n ); 
            CPPUNIT_ASSERT ( n == t );
            CPPUNIT_ASSERT ( t == n );
        }


        void testList() {
            NodeRef n = Node::create("Some Node");
            {
                std::vector<DataType> my_list;
                for (int i= 0; i< 10; ++i ) my_list.push_back ( i );
                my_list.push_back ( "Ten" );
                n->set_attr ( "list_data", my_list );
                // my_list popped
            }
            std::vector<DataType> new_list = n->get_attr ( "list_data" );

            CPPUNIT_ASSERT_EQUAL ( 11, (int32) new_list.size() );
            CPPUNIT_ASSERT_EQUAL ( 7, (int32) new_list.at(7) );
            CPPUNIT_ASSERT_EQUAL ( std::string ( "Ten" ) , (std::string) new_list.at(10) );
            
        }


        void testBigInt() {
            
            std::vector<DataType> ints;
            ints.push_back ( (uint32) 0x12345678 );
            ints.push_back ( (uint32) 0x9abcdef0 );
            DataType bi = DataType::as_bigint_datatype( ints );
            CPPUNIT_ASSERT_EQUAL ( BIGINT_DATA, bi.get_type() );

            DataType bi2 = bi;
            CPPUNIT_ASSERT_EQUAL ( BIGINT_DATA, bi.get_type() );

            std::vector<DataType> ints2 = DataType::as_bigints ( bi2 );

            CPPUNIT_ASSERT_EQUAL ( ints, ints2 );
        }
            

};


CPPUNIT_TEST_SUITE_REGISTRATION ( TypeTest );
