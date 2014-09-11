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
#include <iostream>
#include <fstream>

#include <nitro/xmlwriter.h>
#include <nitro/node.h>
#include <nitro/error.h>


using namespace std;


namespace Nitro {


struct XmlWriter::impl {
  string m_path; 


  impl ( const string& path ) : m_path ( path ) {} 
};

XmlWriter::XmlWriter(const string& path) : m_impl ( new impl ( path ) ) {}
XmlWriter::~XmlWriter() throw () {
    delete m_impl;
}


#define COMMENT( pre, node ) if ( node->has_attr("comment") ) { \
    out << pre << "<comment><![CDATA[" << node->get_attr("comment") << "]]></comment>" << endl; \
    }
#define VALUEMAP(pre, node ) if (node->has_attr("valuemap") ) { \
    out << pre << "<valuemap>" << endl;\
    NodeRef valmap = node->get_attr("valuemap"); \
    for (DIAttrIter itr = valmap->attrs_begin(); itr != valmap->attrs_end(); ++itr ) {\
        out << pre << "  <entry name=\"" << itr->first << "\" value=\"" << itr->second << "\" />" << endl; \
    }\
    out << pre << "</valuemap>" << endl; \
    }

void XmlWriter::write(const NodeRef& node) {


   if ( Node::DEVIF != node->get_type() ) throw Exception ( XML_ENTITY, "Unsupported root node type." ); 
   
   ofstream out;
   out.open( m_impl->m_path.c_str() );

   out << "<deviceinterface" << endl;
   out << "  xmlns=\"http://ubixum.com/deviceinterface/\"" << endl;
   out << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
   out << "  xsi:schemaLocation=\"http://ubixum.com/deviceinterface/ deviceinterface.xsd\"" << endl;
   if ( node->has_attr("name")) {
       out << "  name=\"" << node->get_attr("name") << '"' << endl;
   }
   if ( node->has_attr("version")) {
       out << "  version=\"" << node->get_attr("version") << '"' << endl;
   }
   out << "  >" << endl;

   for (DITreeIter term_itr = node->child_begin(); term_itr != node->child_end(); ++term_itr ) {
        NodeRef term = *term_itr;
        out << "  <terminal " << endl;
        out << "      name=\"" << term->get_name() << '"' << endl; 
        out << "      regAddrWidth=\"" << term->get_attr ( "regAddrWidth" ) << '"' << endl;
        out << "      regDataWidth=\"" << term->get_attr ( "regDataWidth" ) << '"' << endl;
        out << "      addr=\"" << term->get_attr("addr") << '"' << endl;
        if ( term->has_attr("endian") ) {
            out << "      endian=\"" << term->get_attr("endian") << '"' << endl;
        }
        if ( term->has_attr("version") ) {
            out << "      version=\"" << term->get_attr("version") << '"' << endl;
        }
        out << "      >" << endl;

        COMMENT ( "   " , term );

        for (DITreeIter reg_itr = term->child_begin(); reg_itr != term->child_end(); ++reg_itr ) {
           NodeRef reg = *reg_itr;
           out << "    <register" << endl;
           out << "       name=\"" << reg->get_name() << '"' << endl;
           if (reg->has_attr("endian")) {
             out << "       endian=\"" << reg->get_attr("endian") << '"' << endl;
           }
           out << "       addr=\"" << reg->get_attr("addr") << "\">" << endl;
           COMMENT ( "      ", reg );
           out << "       <type>" << reg->get_attr("type") << "</type>" << endl;
           out << "       <mode>" << reg->get_attr("mode") << "</mode>" << endl;
           if ( reg->has_children() ) {
                for (DITreeIter sub_itr = reg->child_begin(); sub_itr != reg->child_end(); ++sub_itr) {
                    NodeRef subreg = *sub_itr;
                    out << "      <subregister" << endl;
                    out << "        name=\"" << subreg->get_name() << "\">" << endl;
                    COMMENT ( "        ", subreg );
                    out << "        <vlog_name>" << subreg->get_attr ( "vlog_name" ) << "</vlog_name>" << endl;
                    out << "        <width>" << subreg->get_attr("width") << "</width>" << endl;
                    out << "        <init>" << subreg->get_attr("init") << "</init>" << endl;
                    VALUEMAP ( "        " , subreg );
                    out << "      </subregister>" << endl;
                }
           } else {
           out << "       <width>" << reg->get_attr("width") << "</width>" << endl;
           out << "       <init>" << reg->get_attr("init") << "</init>" << endl;
           out << "       <array>" << reg->get_attr("array") << "</array>"<< endl;
           VALUEMAP ( "      ", reg );
           }
           

           out << "    </register>" << endl;
        }
         
    
        out << "  </terminal>" << endl;
   } // end terminals

   out << "</deviceinterface>" << endl;
   out.close();
}


} // end namespace

