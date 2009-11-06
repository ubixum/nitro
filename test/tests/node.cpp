
#include <cppunit/extensions/HelperMacros.h>

#include <nitro.h>

using namespace Nitro;

class NodeTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE ( NodeTest );
    CPPUNIT_TEST ( testCreate );
    CPPUNIT_TEST ( testCopy );
    CPPUNIT_TEST ( testAttrs );
    CPPUNIT_TEST ( testOrder );
    CPPUNIT_TEST ( testRefs );
    CPPUNIT_TEST ( testNodeRefPtr );
    CPPUNIT_TEST ( testErase );
    CPPUNIT_TEST ( testNameChange );

    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }
        void tearDown() {
        }
        void testCreate() {
            NodeRef node = DeviceInterface::create ( "test_node" );
            CPPUNIT_ASSERT( node->has_children() == false );
            CPPUNIT_ASSERT_EQUAL( std::string("test_node"), node->get_name() );

            for (int i=0;i<100;++i) {
                NodeRef node = Node::create("test");
                node->set_attr( "some attr", i );
            }
        }
        void testCopy() {

            NodeRef orig = DeviceInterface::create ( "orig" );
            orig->add_child(Terminal::create("child1"));
            orig->add_child(Terminal::create("child2"));
            NodeRef copy(orig);
            
            CPPUNIT_ASSERT ( copy->has_children() );
            CPPUNIT_ASSERT ( std::string("child1") == copy->get_child("child1")->get_name() );
            CPPUNIT_ASSERT ( std::string("child2") == copy->get_child("child2")->get_name() );
            CPPUNIT_ASSERT_THROW ( copy->get_child("child3") , Exception );

            NodeRef orig2 = DeviceInterface::create("orig");
            orig2->add_child(Terminal::create("child3"));
            orig2->add_child(Terminal::create("child4"));
            CPPUNIT_ASSERT_THROW ( orig2->add_child(Terminal::create("child3")), Exception );
            copy = orig2;
            
            CPPUNIT_ASSERT ( copy->has_children() );
            CPPUNIT_ASSERT_THROW ( copy->get_child("child1"), Exception );
            CPPUNIT_ASSERT_EQUAL ( std::string("child3") , copy->get_child("child3")->get_name() );

            int children=0;
            for (DITreeIter i = copy->child_begin(); i != copy->child_end(); ++i ) {
               children += 1; 
            }
            CPPUNIT_ASSERT ( children == 2 );
        };
        void testAttrs() {
           NodeRef orig = DeviceInterface::create("top");

           CPPUNIT_ASSERT_THROW ( orig->get_attr("attrX"), Exception );
           orig->set_attr ( "attr1", 1 );
           orig->set_attr ( "attr2", std::string("two") );
           orig->set_attr ( "attr3", 3 );
           CPPUNIT_ASSERT ( orig->get_attr("attr3")==3 );
           orig->set_attr ( "attr3", 4 );  // overwrite
           CPPUNIT_ASSERT ( orig->get_attr("attr3")==4 );
           orig->set_attr ( "attr3", 5 ); // overwrite again
           CPPUNIT_ASSERT ( orig->get_attr("attr3")==5 );

           CPPUNIT_ASSERT ( orig->get_attr("attr1")== 1 );
           CPPUNIT_ASSERT ( orig->get_attr("attr2") == "two" );
           CPPUNIT_ASSERT_THROW ( orig->get_attr ( "attrX" ), Exception );


            {
               NodeRef attr_map = Node::create("generic attrs");
               attr_map->set_attr ( "map1", 1 );
               attr_map->set_attr ( "map2", 2 );
               orig->set_attr ( "map", attr_map );
               // attr map out of scope
            }

           CPPUNIT_ASSERT_NO_THROW ( orig->get_attr ( "map" ));
           CPPUNIT_ASSERT_EQUAL ( NODE_DATA, orig->get_attr( "map" ).get_type());
           CPPUNIT_ASSERT_EQUAL ( 2, (int32)((NodeRef)orig->get_attr("map"))->get_attr ("map2") );

           
           // four attrs
           DIAttrIter ai = orig->attrs_begin();
           ++ai;
           ++ai;
           CPPUNIT_ASSERT ( ai != orig->attrs_end() );
           ++ai;
           ++ai;
           CPPUNIT_ASSERT ( ai == orig->attrs_end() );

           {
            NodeRef first = Terminal::create("first");
            first->set_attr("attr1", 1 );
            orig->add_child(first);
            //first goes out of scope
           }

           NodeRef child=orig->get_child("first");
           child->set_attr ( "attr2", 2 );
           CPPUNIT_ASSERT_THROW ( orig->get_child ("first")->get_attr ( "attrX" ), Exception );

           CPPUNIT_ASSERT ( orig->get_child("first")->get_attr("attr2") == 2 );

           {
            NodeRef second = Terminal::create("second");
            second->set_attr("attr3",std::string("three"));
            orig->add_child(second);
            // second goes out of scope
           }
           CPPUNIT_ASSERT_NO_THROW ( orig->get_child("second")->get_attr("attr3") );
           NodeRef second=orig->get_child("second");
           CPPUNIT_ASSERT ( second->get_attr("attr3") == "three" );
           CPPUNIT_ASSERT_THROW ( orig->get_child("childY"), Exception );

           // child ref should still be good
           CPPUNIT_ASSERT_NO_THROW ( child->get_attr("attr1") ); 

        }

        void testOrder() {
            NodeRef orig = DeviceInterface::create("top");
            for (int i=10;i>=0;--i) {
                std::stringstream io;
                std::string name;
                io << i;
                io >> name;
                NodeRef child = Terminal::create(name);
                orig->add_child(child);
            }
            // nodes are ordered 
            int i=10;
            for (DITreeIter itr=orig->child_begin();itr!=orig->child_end();++itr) {
               NodeRef c =*itr; 
               std::stringstream io;
               std::string name;      
               io << i--;
               io >> name;
               CPPUNIT_ASSERT_EQUAL ( name, c->get_name() ); 
            }
        }

        void testRefs() {
            NodeRef orig = DeviceInterface::create("top");
            orig->add_child(Terminal::create("child1"));
            NodeRef child2 = Register::create("child2");
            CPPUNIT_ASSERT_NO_THROW(orig->get_child ( "child1")->add_child(child2));
            child2->set_attr("test", "hello" );
            CPPUNIT_ASSERT_NO_THROW(orig->get_child ( "child1")->get_child("child2")->get_attr("test"));
            CPPUNIT_ASSERT_EQUAL ( std::string("hello"), (std::string)orig->get_child("child1")->get_child("child2")->get_attr("test"));
        }

        void testNodeRefPtr() {
            // this usage is causing problems with node destructor in C#
            NodeRef* ptr = new NodeRef();
            *ptr = Node::create("test");
            (*ptr)->set_attr ( "attr", 3 );
            try {
                (*ptr)->get_attr("b"); // throw
                CPPUNIT_FAIL ( "Should have thrown." );
            } catch ( const Exception &) {}
            delete ptr;
        }

        void testErase() {
           NodeRef node = Node::create("Test");
           NodeRef child1 = Node::create("child1");
           NodeRef child2 = Node::create("child2");
           node->add_child(child1);
           node->add_child(child2);
           CPPUNIT_ASSERT_NO_THROW( node->del_child("child2") );
           CPPUNIT_ASSERT_EQUAL ( 1, (int) node->num_children() );
           CPPUNIT_ASSERT_THROW ( node->del_child ( "child2" ), Exception ); 
        }

        void testNameChange() {
            NodeRef p = Node::create("Parent");
            NodeRef c = Node::create("child");

            p->add_child( c );

            c->set_name ( "child1" );
            
            CPPUNIT_ASSERT_THROW ( p->get_child ( "child" ), Exception );
            CPPUNIT_ASSERT_NO_THROW ( p->get_child ( "child1" ) );


        }

};


CPPUNIT_TEST_SUITE_REGISTRATION ( NodeTest );
