
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>

using namespace Nitro;
using namespace std;

class XmlTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( XmlTest );
    CPPUNIT_TEST ( testXml );
    CPPUNIT_TEST ( testTwice );
    CPPUNIT_TEST ( testPaths );
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {
        }

        void testXml () {
            
            const char* xml_path="test.xml";
            XmlReader dummy ( "some_nonexistent_file_path" ) ;
            NodeRef tmp = DeviceInterface::create("hi");
            CPPUNIT_ASSERT_THROW(dummy.read(tmp), Exception);
                        
            XmlReader reader (xml_path, true);
            NodeRef tree = DeviceInterface::create("root");
            CPPUNIT_ASSERT_NO_THROW(reader.read(tree));
 
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child("Terminal1") );
            CPPUNIT_ASSERT_THROW ( tree->get_child("some_other_term"), Exception );
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child("Terminal1")->get_child("reg1"));
 
            NodeRef term1=tree->get_child("Terminal1");
            NodeRef reg1=term1->get_child("reg1");
 
            CPPUNIT_ASSERT_NO_THROW ( reg1->get_attr("width") );
            CPPUNIT_ASSERT ( reg1->get_attr("width") == 18 );
            CPPUNIT_ASSERT_NO_THROW ( reg1->get_attr("mode") );
            CPPUNIT_ASSERT ( reg1->get_attr("mode") == "write" );
            CPPUNIT_ASSERT_NO_THROW ( reg1->get_attr("type") );
            CPPUNIT_ASSERT ( reg1->get_attr("type") == "int" );
 
 
            CPPUNIT_ASSERT_NO_THROW (  term1->get_child("addr2reg") );
            CPPUNIT_ASSERT_NO_THROW ( term1->get_child("addr2reg")->get_attr("addr") );
            CPPUNIT_ASSERT ( term1->get_child("addr2reg")->get_attr("addr") == 2 );
            CPPUNIT_ASSERT ( term1->get_child("addr2reg")->has_attr("comment"));
 
            CPPUNIT_ASSERT_NO_THROW( term1->get_child("reg2") );
            NodeRef reg2=term1->get_child("reg2");
            CPPUNIT_ASSERT ( reg2->get_attr("type") == "trigger" );
            CPPUNIT_ASSERT ( reg2->get_attr("addr") == 23 );
 
            CPPUNIT_ASSERT_NO_THROW( term1->get_child("reg3") );
            NodeRef reg3=term1->get_child("reg3");
            CPPUNIT_ASSERT ( reg3->get_attr("type") == "int" );
            CPPUNIT_ASSERT_NO_THROW ( reg3->get_attr("width") );
            CPPUNIT_ASSERT ( reg3->get_attr("width") == 3 );
            CPPUNIT_ASSERT_NO_THROW ( reg3->get_attr ( "addr" ) );
            CPPUNIT_ASSERT ( reg3->get_attr("addr") == 24 );

            CPPUNIT_ASSERT_NO_THROW ( reg3->get_child("subreg2") );
            NodeRef sub2 = reg3->get_child("subreg2");
            CPPUNIT_ASSERT_NO_THROW ( sub2->get_attr("valuemap") );
            CPPUNIT_ASSERT_EQUAL ( 1, (int32) ((NodeRef)sub2->get_attr("valuemap"))->get_attr("OFF") );
 
 
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child("term2") );
            CPPUNIT_ASSERT ( tree->get_child("term2")->get_attr("addr") == 4 );

            CPPUNIT_ASSERT_NO_THROW ( tree->get_child("array_term" )->get_child("array1") );
            CPPUNIT_ASSERT_EQUAL ( 5, (int) tree->get_child("array_term")->get_child("array1")->get_attr("array") );

            NodeRef a2 = tree->get_child("array_term")->get_child("array2");
            CPPUNIT_ASSERT_EQUAL ( LIST_DATA, a2->get_attr("init").get_type() );
            vector<DataType> init = a2->get_attr("init");
            CPPUNIT_ASSERT_EQUAL ( INT_DATA, init.at(1).get_type() );
            CPPUNIT_ASSERT_EQUAL ( 2, (int) init.at(1) );
            CPPUNIT_ASSERT_EQUAL ( BIGINT_DATA, init.at(2).get_type() ); 


            // include
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child ( "include_term" ) );
            CPPUNIT_ASSERT_EQUAL ( 6, (int) tree->get_child ( "include_term" )->get_attr ( "addr" ) );
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child ( "include_term1" ) );
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child ( "include_term1" )->get_child("reg_renamed"));
            CPPUNIT_ASSERT_EQUAL ( 77, (int) tree->get_child ( "include_term1")->get_attr ( "addr" ) );
            CPPUNIT_ASSERT_EQUAL ( 1, (int) tree->get_child("include_term1")->get_child("reg_renamed")->get_attr("addr") );
            CPPUNIT_ASSERT_NO_THROW ( tree->get_child ("include_term1" )->get_child ("new_register") );
        }

        void testTwice() {
            const char* xml_path="test.xml";
            XmlReader reader ( xml_path, true );
            NodeRef tree = DeviceInterface::create("di");
            reader.read(tree);
            CPPUNIT_ASSERT_NO_THROW ( reader.read(tree) );
        }

        void testPaths() {
            const char* xml_path = "xmldir/test.xml";
            XmlReader reader ( xml_path, true );
            NodeRef di = DeviceInterface::create("di");
            CPPUNIT_ASSERT_NO_THROW( reader.read(di) );
        }


};

CPPUNIT_TEST_SUITE_REGISTRATION ( XmlTest );
