/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <cstdlib> // getenv
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype> // isalnum

#include <nitro/node.h>
#include <nitro/error.h>
#include <nitro/xmlreader.h>

#include "bihelp.h"
#include "xutils.h"

namespace Nitro {


#ifdef DEBUG_NODE
#define node_debug(d) std::cout << "\t\t****" << d << std::endl
#else
#define node_debug(d)
#endif

bool file_exists(const std::string &path) {
   std::ifstream file(path.c_str());
   return file; // file closed if goes out of scope.
}

NodeRef load_di( const std::string &path, NodeRef dst ) {
    // construct list of paths to search

    std::string found_path = path;
    
    if (!file_exists(found_path)) {
        // in this case, try additional
        // paths with the NITRO_DI_PATH var
        char *env = getenv ( "NITRO_DI_PATH" );
        if (env) {
            std::string envpath  ( env ); // don't modify env
            std::istringstream in ( envpath );
            while (in.good()) {
                std::getline(in, envpath, ':');
                if (!in.bad()) {
                    found_path = xjoin ( envpath, path );
                    if (file_exists(found_path)) break;
                }
            }

        } else throw Exception ( PATH_LOOKUP );
    }

    XmlReader reader ( found_path );
    reader.read(dst);
    return dst;
}



// name helper function for di nodes

bool valid_name(const std::string& name) {
   
   if (name.size()<1) return false;
   for (std::string::const_iterator i=name.begin(); i != name.end(); ++i ) {
      if ( !isalnum(*i) && *i != '_' ) return false;
   }
   if (isdigit(name.at(0))) return false;

   return true;
}

//********** Node::impl **************

typedef std::map<std::string,int> childrenmap_t;

struct Node::impl {
	std::string m_name;
	std::vector<NodeRef> m_children;
	childrenmap_t m_childrenmap;
	std::map<std::string,DataType> m_attrmap;
    static int m_global_refs;
    int m_ref_count;
    Node* m_parent;
    void inc() { ++m_ref_count; ++m_global_refs; }
    void dec() { --m_ref_count; --m_global_refs; }

	impl(const std::string &name) : m_name(name), m_ref_count(0), m_parent(NULL) {}

	bool has_child(const std::string &name);
    ~impl() {}
};


int Node::impl::m_global_refs=0;

bool Node::impl::has_child(const std::string &name) {	
	return m_childrenmap.find( name ) != m_childrenmap.end();
}


//********** NodeRef ******************

NodeRef::NodeRef() : m_node ( NULL )  {
    node_debug ( "Tmp Node Creation" );
}

NodeRef::NodeRef ( Node* node ) : m_node ( node ) {
    node_debug ("New Node Ref: " << node->get_name() );
    m_node->m_impl->inc();
}

void NodeRef::dec() throw() {
    if (m_node) {
        m_node->m_impl->dec();
        node_debug ( "Delete NodeRef: " << m_node->get_name() << " remaining refs: " << m_node->m_impl->m_ref_count << " " << m_node);
        if (m_node->m_impl->m_ref_count <=0) {   
            delete m_node;
            node_debug ( "Global NodeRefs: " << Node::impl::m_global_refs );
        }
    }
    m_node=NULL;
}

NodeRef::NodeRef ( const NodeRef& copy ) : m_node (copy.m_node)  {
    if ( m_node ) {
        m_node->m_impl->inc();
        node_debug ( "Copy Node: " << copy->get_name() << " refs: " << m_node->m_impl->m_ref_count );
    }
}
NodeRef::~NodeRef ( ) throw() { 
    dec();
}

bool NodeRef::is_null() const {
 return m_node == NULL;
}

Node* NodeRef::operator ->() const {
    if (!m_node) throw Exception ( NODE_OP_ERROR, "Attempt to use unassigned NodeRef" );
    node_debug ( "Using Pointer " << m_node);
    return m_node;
}
Node& NodeRef::operator *() const {
    if (!m_node) throw Exception ( NODE_OP_ERROR, "Attempt to use unassigned NodeRef" );
    node_debug ( "Using Ref " << m_node );
    return *m_node;
}


NodeRef& NodeRef::operator=( const NodeRef &copy ) {
    if ( this != &copy ) {
        dec();
        m_node = copy.m_node;
        if (m_node) {
           m_node->m_impl->inc();
           node_debug ( "Inc ref: " << m_node->get_name() << " refs: " << m_node->m_impl->m_ref_count );
        }
    }
    return *this;
}


bool NodeRef::operator==( const NodeRef& other ) {
    return m_node == other.m_node;
}


std::ostream& operator << ( std::ostream& out, const NodeRef& n ) {
    out << "NodeRef<NodePtr:" << n.m_node ;
    return out;
}

//********** Node *********************

Node::Node(const std::string &name) : m_impl(new impl(name)) {
	
}

NodeRef Node::create ( const std::string& name ) {
    node_debug ( "Create Generic Node: " << name );
    return NodeRef ( new Node ( name ) );
}

NodeRef Node::clone () const {

    // children, childrenmap, attrmap
    NodeRef copy = this->call_create ( get_name() );
    for ( DITreeIter itr = this->child_begin(); itr != this->child_end(); ++itr ) {
        NodeRef child = (*itr)->clone();
        copy->add_child ( child );
    }

    for ( DIAttrIter itr = this->attrs_begin(); itr != this->attrs_end(); ++itr ) {
        if ( itr->second.get_type() == NODE_DATA ) {
            copy->set_attr( itr->first, ((NodeRef)(itr->second))->clone() );
        } else {
            copy->set_attr( itr->first, itr->second );
        }
    }

    return copy;

}

bool Node::has_children()  const { return !m_impl->m_children.empty(); }


NodeRef Node::get_child ( const std::string& name ) const {
    if (!m_impl->has_child(name)) throw Exception ( NODE_NOT_FOUND, name );
    return m_impl->m_children[m_impl->m_childrenmap[name]];
}

bool Node::has_child ( const std::string& name ) const {
    return m_impl->has_child(name);
}


Node::~Node() throw() { 
    node_debug ( "Deleting Real Node " << std::hex << this );
    delete m_impl;
}

const std::string& Node::get_name() const { return m_impl->m_name; }

void Node::set_name(const std::string& name) {
    std::string orig_name = m_impl->m_name;
    m_impl->m_name = name;
    if (m_impl->m_parent) {
        childrenmap_t::iterator itr = m_impl->m_parent->m_impl->m_childrenmap.find ( orig_name );
        int pos = itr->second;
        m_impl->m_parent->m_impl->m_childrenmap.erase(itr);
        m_impl->m_parent->m_impl->m_childrenmap[name] = pos;
    }
}


void Node::add_child(const NodeRef& node) {
	
    node_debug ( get_name() << ": " << "add_child(" << node->get_name() << ")" );
	// use child keys to determine if key is unique
	if (m_impl->has_child(node->get_name())) {
		throw Exception( NODE_DUP_CHILD, node->get_name() );
	}
    if (node->m_impl->m_parent) {
        throw Exception( NODE_LINKED, node->get_name() );
    }
    if(this == &(*node)) {
        throw Exception ( NODE_SELFREF, node->get_name() );
    }
	// child in order add to vector
	m_impl->m_children.push_back(node);
	m_impl->m_childrenmap[node->get_name()] = m_impl->m_children.size()-1;
    node->m_impl->m_parent = this; 
}

void Node::del_child(const std::string& name) {
    node_debug ( get_name() << ": " << "del_child(" << name << ")" );
    if (!m_impl->has_child(name))
        throw Exception ( NODE_NOT_FOUND, name ); 

    NodeRef child = m_impl->m_children.at(m_impl->m_childrenmap[name]);

    child->m_impl->m_parent = NULL;
    m_impl->m_children.erase ( 
        m_impl->m_children.begin() + 
        m_impl->m_childrenmap[name] );

    m_impl->m_childrenmap.clear();
    for (int i=0;i<m_impl->m_children.size();++i) 
        m_impl->m_childrenmap[m_impl->m_children.at(i)->get_name()] = i;
    
}

uint32 Node::num_children() const {
    return m_impl->m_children.size();
}

bool Node::has_attr( const std::string& name) const {
    return m_impl->m_attrmap.find(name) != m_impl->m_attrmap.end();
}

DataType Node::get_attr(const std::string& name) const {
    node_debug ( get_name() << ": get_attr(" << name << ")" );
    // The old method caused a memory corruption in the attrmap
    // it only occurred in C# bindings and when an invalid attr name
    // was tested for.
    if (!has_attr(name)) throw Exception ( NODE_ATTR_NOT_FOUND,  name );
    return m_impl->m_attrmap.find(name)->second;
}

uint32 Node::num_attrs() const {
    return m_impl->m_attrmap.size();
}


void Node::set_attr(const std::string& name, const DataType& value) {
    node_debug ( get_name() << ": set_attr(" << name << ", " << value << ")" );
    m_impl->m_attrmap.erase(name);
	m_impl->m_attrmap.insert( make_pair( name, value ) );
}


void Node::del_attr(const std::string &name) {
    m_impl->m_attrmap.erase(name);
}

DITreeIter Node::child_begin() const { return m_impl->m_children.begin(); }
DITreeIter Node::child_end() const { return m_impl->m_children.end(); }


DIAttrIter Node::attrs_begin() const { return m_impl->m_attrmap.begin(); }
DIAttrIter Node::attrs_end() const { return m_impl->m_attrmap.end(); }


std::ostream& operator << ( std::ostream& out, const Node& n ) {
    out << "{name: " << n.m_impl->m_name;
    for (DIAttrIter itr = const_cast<Node&>(n).attrs_begin();
                    itr != const_cast<Node&>(n).attrs_end();
                    ++itr ) {
        out << ", " << itr->first << ": " << itr->second;
    }
    if ( n.has_children () ) {
        out << ", children: [";
        for (DITreeIter itr = const_cast<Node&>(n).child_begin();
                       itr != const_cast<Node&>(n).child_end();
                       ++itr ) {
            out << *itr ; 
        }
        out << "]";
    }
    out << "}";
    return out;
}

// for di nodes, this can check the children names
void check_name(const std::string &name) {
    if (!valid_name(name)) throw Exception ( NODE_NAME_ERROR, name );
}


//*********************** Device Interface ***********************

DeviceInterface::DeviceInterface( const std::string& name ) : Node (name ) {
  check_name(name);
}

NodeRef DeviceInterface::create ( const std::string& name ) {
    node_debug ( "Create Device Interface: " << name );
    return NodeRef ( new DeviceInterface ( name ) );
}

void DeviceInterface::set_name ( const std::string& name ) {
    check_name(name);
    Node::set_name(name);
}

void DeviceInterface::add_child ( const NodeRef& node ) {
    // a valid terminal has:
    // addr
    // regAddrWidth
    // regDataWidth 
    if ( TERMINAL != node->get_type() ) throw Exception ( NODE_UNSUPPORTED_CHILD, node->get_name());
    if ( !node->has_attr( "regAddrWidth" ) ) throw Exception ( NODE_ATTR_ERROR , "Terminal node missing regAddrWidth:", node->get_name() );
    if ( !node->has_attr( "regDataWidth" ) ) throw Exception ( NODE_ATTR_ERROR, "Terminal node missing regDataWidth: ", node->get_name() );

    if (node->has_attr("addr")) {
        uint32 node_addr = node->get_attr("addr");
        for ( DITreeIter itr = child_begin(); itr != child_end(); ++itr ) {
           uint32 child_addr = (*itr)->get_attr("addr");
           if (node_addr == child_addr) throw Exception ( NODE_ATTR_ERROR, "Terminal address already exists in device interface.", node_addr );
        }
    } else {
        // auto assigned addresses start at 0x200
        uint32 next_addr = 0x200;
        while (true) {
            bool inc = false;
            for (DITreeIter itr = child_begin(); itr != child_end(); ++itr ) {
                if (next_addr == (uint32)(*itr)->get_attr("addr") ) {
                    inc=true;
                    break;
                }
            }
            if (inc) {
                ++next_addr;
            } else {
                node->set_attr("addr", next_addr );
                break;
            }
        }
    }

    if ( !node->has_attr("comment") ) {
        node->set_attr("comment", "");
    }

    Node::add_child ( node );
}


//********************* Terminal *********************************
NodeRef Terminal::create ( const std::string& name ) {
    node_debug ( "Create Terminal: " << name );
    return NodeRef ( new Terminal ( name ) );
}


void Terminal::add_child ( const NodeRef& node ) {

    if ( REGISTER != node->get_type() ) throw Exception ( NODE_UNSUPPORTED_CHILD , node->get_name() );
    if ( !node->has_attr ( "type" ) ) throw Exception ( NODE_ATTR_ERROR, std::string("Missing type attribute for Register: ") + node->get_name() );
    if ( node->get_attr ( "type" ) != "int" && node->get_attr("type") != "trigger" )
        throw Exception (  NODE_ATTR_ERROR, std::string("Invalid type for Register: ") + (std::string)node->get_attr("type") );

    if ( !node->has_attr("addr") ) {
        // determine the next address
        uint32 next_addr = 0;
        if ( has_children() ) {
            DITreeIter itr = child_end()-1;
            uint32 width = (*itr)->get_attr("width");
            uint32 last_addr = (*itr)->get_attr("addr");
            if ( !has_attr("regDataWidth" ) ) throw Exception ( NODE_ATTR_ERROR , std::string("Terminal node missing regDataWidth: ")+ get_name() );
            uint32 data_width = get_attr("regDataWidth");

            uint32 num_regs =  width/data_width + (width % data_width > 0 ? 1 : 0);
            num_regs *= (uint32)(*itr)->get_attr("array");
            next_addr = last_addr + num_regs;
        }
        node->set_attr("addr", next_addr);
    }

    if ( !node->has_attr("array")) {
        node->set_attr("array", 1);
    }

    if ( !node->has_attr("shadowed") ) {
        node->set_attr("shadowed",0);
    }

    // register addresses are the width of the terminal by default. 
    if ( !node->has_attr("width") ) {
        node->set_attr ( "width" , get_attr ( "regDataWidth" ) );
    }

    // leaves init out when subregisters present
    if ( !node->has_attr("init") ) {
        if (node->has_children()) {
            uint32 width=0;
            mpz_class init(0);

            for (DITreeIter itr=node->child_begin(); itr != node->child_end(); ++itr ) {
                DataType sinit = (*itr)->get_attr("init"); 

                mpz_class i; 
                if ( STR_DATA == sinit.get_type() ) {
                    // convert valuemap
                    NodeRef valuemap = (*itr)->get_attr("valuemap");
                    i = bi_from_dt ( valuemap->get_attr ( (std::string) sinit ) );
                } else {
                    i = bi_from_dt ( sinit );
                    
                }
                i <<= width;
                init |= i;
                width += (uint32) (*itr)->get_attr("width");
            }
            if ( width != (uint32) node->get_attr("width") ) throw Exception ( NODE_ATTR_ERROR, "Incorrect register width calculation");
            node->set_attr("init", dt_from_bi(init) );
        } else {
            node->set_attr("init",0);
        }
    }

    if ( !node->has_attr("mode") ) {
        node->set_attr("mode", "write" );
    }
    std::string mode = node->get_attr("mode");
    if (mode != "read" && mode != "write") throw Exception ( NODE_ATTR_ERROR, std::string("mode must be read or write: ") + node->get_name() + "(" + mode + ")" );

    if ( !node->has_attr("comment") ) {
        node->set_attr("comment", "");
    }

    
    Node::add_child ( node );
}


//*********************** Register ******************************
NodeRef Register::create ( const std::string& name ) {
    node_debug ( "Create Register: " << name );
    return NodeRef ( new Register ( name ) );
}


void Register::add_child ( const NodeRef& node ) {

    if ( SUBREGISTER != node->get_type() ) throw Exception ( NODE_UNSUPPORTED_CHILD, std::string("Cannot add incorrect child type to register: ")+node->get_name() );

    uint32 width=0;
    if (has_attr("width")) width = get_attr("width");
    node->set_attr("addr", width ); // offset of current subreg is previous width of register.

    uint32 sub_width = (uint32) node->get_attr("width");
    if ( sub_width == 0 ) {
        throw Exception ( NODE_ATTR_ERROR, "Invalid width on subregister.", node->get_name() );
    }

    set_attr("width",width+sub_width);

    if (!node->has_attr ( "vlog_name" )) {
        node->set_attr ( "vlog_name", get_name() + "_" + node->get_name() );
    }

    if (has_attr("init")) {
        throw Exception ( NODE_ATTR_ERROR, "Invalid to set init in register and subregisters both.", node->get_name());
    }
    if (!node->has_attr ( "init" )) {
        node->set_attr ( "init" , 0 );
    }

    if ( !node->has_attr("comment") ) {
        node->set_attr("comment", "");
    }


    Node::add_child ( node );
}


//******************** Subregister *****************************
NodeRef Subregister::create ( const std::string& name ) {
    node_debug ( "Create Subregister: " << name );
    return NodeRef ( new Subregister ( name ) );
}


//************************ Valuemap ********************************

NodeRef Valuemap::create ( const std::string& name ) {
    node_debug ( "Create Valuemap: " << name );
    return NodeRef ( new Valuemap ( name ) );
}

} // end namespace
