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

#include <memory>
#include <iostream>
#include <sstream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>


#include <nitro/xmlreader.h>
#include <nitro/error.h>
#include <nitro/node.h>

#include "bihelp.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;
using namespace Nitro;

#ifdef DEBUG_XML
#define debug(x) cout << x << " (" __FILE__ ":" << __LINE__ << ")" << endl;
#else
#define debug(x)
#endif


string& trim ( string& s ) {
    static string ws = "\t\n\n ";
    size_t startpos = s.find_first_not_of (ws);
    size_t endpos = s.find_last_not_of (ws);
    if (string::npos == startpos || string::npos == endpos ) s = "";
    else s=s.substr( startpos, endpos-startpos+1 );

    return s;
}

string to_string( const XMLCh* ent ) {
    char* str = XMLString::transcode(ent);
    string res(str);
    XMLString::release(&str);
    return trim(res);
}



bool isnumeric ( const string& s ) {
    if (!s.size()) return false;
    char first=s.at(0);
    if (s.size()==1 && !isdigit(first)) return false;
    if (first != '+' && first != '-' && !isdigit(first)) return false;
    for (size_t pos=1;pos<s.size();++pos) {
        if (!isdigit(s.at(pos))) return false;
    }
    return true;
}


ostream& operator<<(ostream& out, const XMLCh* const ent ) {
    out << to_string(ent);
    return out;
}



namespace Nitro {

struct XmlReader::impl {
//    std::istream &m_in;
    string m_path;
    bool m_validate;
    impl(const string& path, bool validate) : m_path(path), m_validate(validate) {}

};

class NitroErrorHandler : public DOMErrorHandler {
  private:
   string error;
   bool err;
   void operator=(const NitroErrorHandler&); // unimpled
   NitroErrorHandler(const NitroErrorHandler&); // unimpled
  public:
   NitroErrorHandler() : err(false) {}
   bool handleError ( const DOMError &domerr ){
     stringstream errstr ;
     errstr << to_string ( domerr.getMessage() ); 
     errstr << " (Line: " << domerr.getLocation()->getLineNumber() << ", Column: " << 
                domerr.getLocation()->getColumnNumber() << ")";
     error = errstr.rdbuf()->str();
     err=true;
     return false; 
   }
   bool err_occurred() { return err; }
   string get_message () { return error; }
};


class StringHelper {
        const XMLCh* m_str;
    public:
        StringHelper( const char* str ) : m_str(XMLString::transcode(str)) {}
        ~StringHelper() { XMLString::release ( const_cast<XMLCh**>(&m_str)); }
        int compare ( const XMLCh* str ) const {
            return XMLString::compareIStringASCII( m_str, str );
        }
        operator const XMLCh*() { return m_str; }
};

bool has_attr( DOMNode *node, const char* attr ) {
 StringHelper sh (attr);
 return node->getAttributes() != NULL && node->getAttributes()->getNamedItem(sh) != NULL;
}

DataType from_xmlch ( const XMLCh* src, DATA_TYPE t ) {
    switch (t) { 
        case INT_DATA:
           return (int32) XMLString::parseInt(src) ;
        case UINT_DATA:
           return (uint32) XMLString::parseInt(src) ;
        case STR_DATA:
           return to_string(src);
        default:
           throw Exception(XML_ENTITY, "Unsupported Data Type."); 
    }
}

DataType get_attr( DOMNode *node, const char* attr, DATA_TYPE t ) {
    StringHelper sh ( attr );
    DOMNamedNodeMap *attr_map = node->getAttributes();
    if (!attr_map) throw Exception ( XML_ENTITY, "No attributes for node: " + to_string(node->getNodeName()));
    DOMNode* attr_node = attr_map->getNamedItem( sh );
    if (!attr_node) throw Exception ( XML_INVALID, "Missing attribute for node: " + to_string ( node->getNodeName() ) +", " + attr );

    const XMLCh *attr_val = attr_node->getNodeValue();
    return from_xmlch(attr_val, t);

}

DataType get_node_text ( DOMNode* node, DATA_TYPE t ) {
    DOMNode *text = node->getFirstChild();
    if (NULL!=text && (text->getNodeType() != DOMNode::TEXT_NODE || text->getNodeType() != DOMNode::CDATA_SECTION_NODE)) { 
        const XMLCh *val = text->getNodeValue();
        return from_xmlch(val,t);
    } else {
        cout << " unable to find comment from node" << endl;
        return string ( "" );
    }

}


void handle_valuemap ( NodeRef dimap, DOMNode *valuemap ) {
     for (DOMNode *entry = valuemap->getFirstChild(); NULL != entry; entry = entry->getNextSibling() ) {
        string node_name = to_string(entry->getNodeName());
        if ( node_name == "entry" ) {
            debug("        entry " << get_attr( entry, "name", STR_DATA ) << " " << get_attr ( entry, "value", STR_DATA ) );
            dimap->set_attr ( get_attr ( entry, "name", STR_DATA ), get_attr ( entry, "value", UINT_DATA ) );
        } else {
            if (entry->getNodeType() != DOMNode::TEXT_NODE)
                throw Exception ( XML_INVALID, "Unexpected child of valuemap: " + node_name );
        }
     }
}

void handle_init ( NodeRef reg, const string &init_str) {
    string init = init_str;
    vector<string> array;
    do {
        trim(init);
        size_t pos = init.find(",");
        if (string::npos == pos) {
            array.push_back ( init );
        } else {
            string elem = init.substr(0,pos);
            init=init.substr(pos+1); // skip the comma
            array.push_back ( trim(elem) );
        }
    } while ( init.find(",") != string::npos );

    vector<DataType> arraydt;
    for (vector<string>::iterator itr = array.begin();
             itr != array.end();
             ++itr ) {
             
        string elem = *itr; 
        if ( isnumeric(elem) ) {
            // numeric
            mpz_class ielem ( elem ); // for bit int support
            arraydt.push_back ( dt_from_bi ( ielem ) ); 
        } else {
            // string init values are resolved when added to the di 
            arraydt.push_back(elem);
        } 

    }

    DataType initdt(0);
    if (arraydt.size()==1) initdt = arraydt.at(0);
    else initdt=arraydt;

    reg->set_attr("init", initdt );
}

void handle_subreg( NodeRef disubreg, DOMNode *subreg ) {
    
    for (DOMNode *subchild = subreg->getFirstChild(); NULL != subchild; subchild = subchild->getNextSibling() ) {
        string node_name = to_string ( subchild->getNodeName() );
        debug("        <" << node_name << " ..." );
        if ( node_name == "comment" ) {
             disubreg->set_attr("comment", get_node_text ( subchild, STR_DATA ) );
        } else if (node_name == "valuemap") {
             NodeRef valuemap = Valuemap::create("valuemap");
             handle_valuemap ( valuemap, subchild ); 
             disubreg->set_attr("valuemap", valuemap );
        } else if (node_name == "width" ) {
            disubreg->set_attr("width", get_node_text ( subchild, UINT_DATA ) ); 
        } else if (node_name == "init" ) {
            handle_init ( disubreg, get_node_text ( subchild, STR_DATA ) );
        } else {
              if (subchild->getNodeType() != DOMNode::TEXT_NODE )
                throw Exception ( XML_INVALID, "Unexpected child of subregister: " + node_name );
        }
        debug("        />");
    }

}

void handle_register ( NodeRef direg, DOMNode *reg ) {
    if (has_attr(reg,"addr")) {
       direg->set_attr("addr", get_attr(reg,"addr",UINT_DATA));
    }
    for (DOMNode *regchild = reg->getFirstChild(); NULL != regchild; regchild = regchild->getNextSibling() ) {
          string nodename = to_string ( regchild->getNodeName() );
          if ( nodename == "type" ) {
             // NOTE check int/trigger or better move validation to direg 
             direg->set_attr("type", get_node_text ( regchild, STR_DATA ) );
          } else if (nodename == "mode") {
             direg->set_attr("mode", get_node_text ( regchild, STR_DATA ) );
          } else if (nodename == "subregister") {
             NodeRef subreg = Subregister::create ( get_attr ( regchild, "name", STR_DATA ) ); 
             debug("      <subreg name=\"" << subreg->get_name() << '"');             
             handle_subreg ( subreg, regchild );
             direg->add_child ( subreg );
             debug("      />");
          } else if (nodename == "array") {
             direg->set_attr("array", get_node_text( regchild, UINT_DATA ) );
          } else if (nodename == "init" ) {
             handle_init ( direg, get_node_text( regchild, STR_DATA ) );
          } else if (nodename == "valuemap" ) {
             NodeRef valuemap = Valuemap::create("valuemap");
             handle_valuemap ( valuemap, regchild ); 
             direg->set_attr("valuemap", valuemap );
          } else if (nodename == "width" ) {
             direg->set_attr("width", get_node_text ( regchild, UINT_DATA ) );
          } else if (nodename == "comment" ) {
             direg->set_attr("comment", get_node_text ( regchild, STR_DATA ) );
          } else {
              if (regchild->getNodeType() != DOMNode::TEXT_NODE )
                throw Exception ( XML_ENTITY, "Unexpected child node of register: " + nodename );
          }
        
    }
}


void handle_registers ( NodeRef diterm , DOMNode *term ) {
     for (DOMNode *reg = term->getFirstChild(); NULL != reg; reg = reg->getNextSibling() ) {
        string node_name = to_string(reg->getNodeName());
        if ("comment" == node_name ) {
           // not a reg
           diterm->set_attr("comment", get_node_text ( reg, STR_DATA ) );
        } else if ("register" == node_name ) {
            NodeRef direg = Register::create ( get_attr(reg, "name", STR_DATA ) );
            debug("    <register name=\"" << direg->get_name() ); 
            handle_register ( direg, reg );
            diterm->add_child ( direg );
            debug("    />");
           
        } else {
            if (reg->getNodeType() != DOMNode::TEXT_NODE )
                throw Exception ( XML_INVALID, "Unexpected child node of terminal: " + node_name );
        }
    }
 
}

void handle_terminals ( NodeRef dest, DOMNode *di ) {

  debug ("<deviceinterface>");
  for ( DOMNode *term = di->getFirstChild(); NULL != term; term = term->getNextSibling() ) {
     string node_name = to_string ( term->getNodeName() ); 

     if ( node_name == "deviceinterface" ) {
        // allows more terminals embedded in a device interface
        handle_terminals ( dest, term );
     } else if ( node_name == "terminal" ) {
        NodeRef diterm = Terminal::create( get_attr(term, "name", STR_DATA) );
        debug("  <terminal name=\"" << diterm->get_name() );
        diterm->set_attr ( "regDataWidth", get_attr(term, "regDataWidth", UINT_DATA ) );
        diterm->set_attr ( "regAddrWidth", get_attr(term, "regAddrWidth", UINT_DATA ) );
        if (has_attr(term,"addr")) {
          diterm->set_attr("addr", get_attr(term, "addr", UINT_DATA ));
        }
       
        handle_registers ( diterm, term ); 
        debug("  />");

        if (dest->has_child ( diterm->get_name() ) ) dest->del_child ( diterm->get_name() );
        dest->add_child ( diterm );

     } else {
        if ( term->getNodeType() != DOMNode::TEXT_NODE )
            throw Exception ( XML_INVALID, "Invalid child node deviceinterface: " + node_name ); 
     }

  }
  debug("/>");
}



XmlReader::XmlReader(const std::string& file_path, bool validate) : m_impl(new impl(file_path, validate)) {
}
XmlReader::~XmlReader() throw() { delete m_impl;}


struct XmlUnitializer {
    DOMLSParser* parser;
    XmlUnitializer(DOMLSParser *p) : parser(p) {}
    ~XmlUnitializer() {
        if (parser) parser->release(); // should be done before terminate
        XMLPlatformUtils::Terminate();
    }
};

void XmlReader::read(NodeRef node) {

    try {
        XMLPlatformUtils::Initialize();
    } catch ( const XMLException& err ) {
		string cause = to_string(err.getMessage());
        throw Exception ( XML_INIT, cause);
    }

    // terminate xml when function exits
    XmlUnitializer xml_uninitializer(NULL);
    
    // get the DOM LS parser
    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, sizeof(tempStr)-1 );
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);

    DOMLSParser* parser = static_cast<DOMImplementationLS*>(impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    xml_uninitializer.parser=parser; // ensure delete parser before Terminate

    DOMConfiguration *dc = parser->getDomConfig();

    NitroErrorHandler handler;
    dc->setParameter ( XMLUni::fgDOMErrorHandler, &handler);
    dc->setParameter ( XMLUni::fgDOMComments, false );
    dc->setParameter ( XMLUni::fgDOMElementContentWhitespace, false );
    dc->setParameter ( XMLUni::fgDOMNamespaces, true );
    if ( m_impl->m_validate ) {
        dc->setParameter ( XMLUni::fgDOMValidate, true );
        dc->setParameter ( XMLUni::fgXercesSchema, true );
        dc->setParameter ( XMLUni::fgXercesSchemaFullChecking, true );
    }
    if (dc->canSetParameter(XMLUni::fgXercesDoXInclude, true)){
          dc->setParameter(XMLUni::fgXercesDoXInclude, true);
    } else {
       throw Exception ( XML_INIT, "XInclude functionality not available." ); 
    }

    try {
       DOMDocument *doc = parser->parseURI(m_impl->m_path.c_str()); 
       if ( handler.err_occurred() ) {
         throw Exception ( XML_PARSE, handler.get_message() );
       }

       DOMNode* root = doc->getDocumentElement();
       root->normalize(); // gets rid of empty text nodes etc.
       handle_terminals ( node, root );
       
    } catch ( const XMLException& e ) {
        //cout << e.getMessage() << endl;
        stringstream s;
        s << to_string(e.getMessage());
        s << '(' << e.getSrcFile() << ":" << e.getSrcLine() << ')';
        throw Exception ( XML_PARSE, s.rdbuf()->str() );
    } catch ( const DOMException& e ) {
        //cout << e.getMessage() << endl;
        throw Exception ( XML_PARSE, to_string(e.getMessage()) );
    } 
}




} // end namespace

