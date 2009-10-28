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


#include <string>

#include "node.h"
#include "util.h"
#include "error.h"

namespace Nitro{
namespace NET{

	Node::Node(Nitro::NodeRef& node): m_node(new Nitro::NodeRef(node)) {}

	Node::Node(String^ name) {
		std::string n=to_string(name);
		m_node = new Nitro::NodeRef();        
		*m_node= Nitro::Node::create( n );
	}
	Node::Node(const Node^ copy) {
		m_node=new Nitro::NodeRef(*(copy->m_node));
	}
    Node::!Node() {
		delete m_node;
	}
	
	Node^ Node::GetChild(const String^ name) {
		std::string n=to_string(name);
		try {
			Nitro::NodeRef node=(*m_node)->get_child(n);
			return gcnew Node(node);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
		
	}

    ArrayList^ Node::ChildrenNames() {
        
        try {
            ArrayList^ names  = gcnew ArrayList();                        
            for (DITreeIter itr = (*m_node)->child_begin(); itr != (*m_node)->child_end(); ++itr ) {
                Nitro::NodeRef child = *itr;
                String^ name = gcnew String( child->get_name().c_str() );
                names->Add(name);
            }
            return names;
        } catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
    }

	void Node::AddChild( const Node^ child) {
		try {
			(*m_node)->add_child ( *(child->m_node) );
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

	bool Node::HasAttr( const String^ name ) {
		std::string n = to_string(name);
		try {
			return (*m_node)->has_attr(n);
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}

    ArrayList^ Node::AttrNames() {
        
        try {
            ArrayList^ names  = gcnew ArrayList();                        
            for (DIAttrIter itr = (*m_node)->attrs_begin(); itr != (*m_node)->attrs_end(); ++itr ) {
                std::string a = itr->first;
                String^ name = gcnew String( a.c_str() );
                names->Add(name);
            }
            return names;
        } catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
    }

	const DataType^ Node::GetAttr(const String^ name) {
		std::string n=to_string(name);
		try {
			const Nitro::DataType& attr = (*m_node)->get_attr( n );
            return gcnew DataType ( attr );			
		} catch ( Nitro::Exception &e ) {
			throw static_cast<NitroException^>(e);
		}
		
	}
	void Node::SetAttr(const String^ name, const DataType^ value ) {
		std::string n=to_string(name);
		try {
			(*m_node)->set_attr( n, (Nitro::DataType)value );
		} catch ( Nitro::Exception &e) {
			throw static_cast<NitroException^>(e);
		}
	}

}} // end namespaces
