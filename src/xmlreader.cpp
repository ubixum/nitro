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

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>


#include <nitro/xmlreader.h>
#include <nitro/error.h>
#include <nitro/node.h>

#include "bihelp.h"
#include "xutils.h"

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


class NitroErrorHandler : public HandlerBase {
  private:
   string m_error;
   bool err;
   void operator=(const NitroErrorHandler&); // unimpled
   NitroErrorHandler(const NitroErrorHandler&); // unimpled
  public:
   NitroErrorHandler() : err(false) {}
   void error ( const SAXParseException &domerr ){
     stringstream errstr ;
     errstr << to_string ( domerr.getMessage() ); 
     errstr << " (Line: " << domerr.getLineNumber() << ", Column: " << 
                domerr.getColumnNumber() << ")";
     m_error = errstr.rdbuf()->str();
     err=true;
   }
   void fatalError ( const SAXParseException &e ) { error(e); }
   bool err_occurred() { return err; }
   string get_message () { return m_error; }
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
    if (has_attr(reg,"endian")) {
       direg->set_attr("endian", get_attr(reg,"endian", STR_DATA ));
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

void handle_terminal ( NodeRef di, DOMNode *term ) {
    NodeRef diterm = Terminal::create( get_attr(term, "name", STR_DATA) );
    debug("  <terminal name=\"" << diterm->get_name() );
    diterm->set_attr ( "regDataWidth", get_attr(term, "regDataWidth", UINT_DATA ) );
    diterm->set_attr ( "regAddrWidth", get_attr(term, "regAddrWidth", UINT_DATA ) );
    if (has_attr(term,"endian")) {
      diterm->set_attr("endian", get_attr(term, "endian", STR_DATA ) );
    }
    if (has_attr(term,"addr")) {
      diterm->set_attr("addr", get_attr(term, "addr", UINT_DATA ));
    }
    if (has_attr(term,"version")) {
        diterm->set_attr( "version", get_attr( term, "version", STR_DATA ) );
    }
   
    handle_registers ( diterm, term ); 
    debug("  />");

    if (di->has_child ( diterm->get_name() ) ) di->del_child ( diterm->get_name() );
    di->add_child ( diterm );
}


void handle_regover ( NodeRef term, DOMNode *overlay ) {
    string name = get_attr ( overlay, "name", STR_DATA );
    NodeRef origreg = term->get_child( name );
    if (has_attr(overlay, "newname" )) {
        origreg->set_name( get_attr ( overlay, "newname", STR_DATA ) );
    }
    if (has_attr(overlay, "addr")) {
        origreg->set_attr( "addr", get_attr ( overlay, "addr", UINT_DATA ) );
    }
}


void handle_termoverlay ( NodeRef di, DOMNode *overlay ) {
    string name = get_attr ( overlay, "name", STR_DATA );
    NodeRef origterm = di->get_child ( name );
    if (has_attr ( overlay, "newname" )) {
        string newname = get_attr ( overlay, "newname", STR_DATA );
        origterm->set_name(newname);
    }
    if (has_attr ( overlay, "addr" )) {
        origterm->set_attr("addr", get_attr(overlay,"addr",UINT_DATA));
    }

    for (DOMNode *regover = overlay->getFirstChild();
         NULL != regover;
         regover = regover->getNextSibling() ) {
        string node_name = to_string(regover->getNodeName());
        if ( node_name == "register" ) {
            NodeRef reg = Register::create( get_attr ( regover, "name", STR_DATA ) ); 
            handle_register ( reg, regover );
            origterm->add_child( reg );
        } else if ( node_name == "regoverlay" ) {
            handle_regover ( origterm, regover );
        }
    }
}

void handle_include ( NodeRef dest, DOMNode *include, bool validate, string orig_path ) {
     string src = get_attr ( include, "src", STR_DATA );
     NodeRef includedi = DeviceInterface::create("includedi");
     string unique_fake_term_name = "fake_addr_term879873498723947293797998749527"; 
     if ( dest->has_children() ) {
        /**
         * To make sure the included terminals are 
         * auto-number (address) from the correct 
         * address number, we need a fake terminal
         **/
        NodeRef fake_addr_term = Terminal::create(unique_fake_term_name);
        NodeRef last_real_term = *(dest->child_end()-1);
        fake_addr_term->set_attr("addr", last_real_term->get_attr("addr"));
        includedi->add_child(fake_addr_term);
     }
     
     // resolve src path to abs path
     string absdir = xdirname ( orig_path );
     string abspath = xjoin ( absdir , src );

     XmlReader xmlr ( abspath, validate );
     xmlr.read ( includedi );

     // overlay/add items
     for ( DOMNode *overlay = include->getFirstChild(); 
           NULL != overlay;
           overlay= overlay->getNextSibling () ) {
           string node_name = to_string ( overlay->getNodeName() );
           if ( node_name == "termoverlay" ) {
               handle_termoverlay ( includedi, overlay ); 
           } 
           // else don't know how to handle element
     }

     for ( DITreeIter itr = includedi->child_begin(); itr != includedi->child_end(); ++itr ) {
         NodeRef iterm = (*itr)->clone();
         if (iterm->get_name() != unique_fake_term_name ) {
             if (dest->has_child( iterm->get_name() ) ) dest->del_child ( iterm->get_name() );
             dest->add_child(iterm);
         }
     }
}

void handle_terminals ( NodeRef dest, DOMNode *di, bool validate, string orig_path ) {


  string dest_name = to_string ( di->getNodeName() ) ;
  if (dest_name == "deviceinterface") {
      debug ("<deviceinterface>");
      if (has_attr(di,"version") ) {
         dest->set_attr("version", get_attr(di,"version",STR_DATA));
      }
      if (has_attr(di,"name") ) {
         dest->set_attr("name", get_attr(di,"name",STR_DATA));
      }
      for ( DOMNode *term = di->getFirstChild(); NULL != term; term = term->getNextSibling() ) {
         string node_name = to_string ( term->getNodeName() ); 
         if ( node_name == "terminal" ) {
            handle_terminal(dest, term); 
         } else if ( node_name == "include" ) {
            handle_include ( dest, term, validate, orig_path );
         } else {
            if ( term->getNodeType() != DOMNode::TEXT_NODE )
                throw Exception ( XML_INVALID, "Invalid child node deviceinterface: " + node_name ); 
         }
    
      }
      debug("/>");
  } else if (dest_name == "terminal") {
      debug ("<terminal>");
      handle_terminal(dest, di); 
      debug ("</terminal>");
  }
}



XmlReader::XmlReader(const std::string& file_path, bool validate) : m_impl(new impl(file_path, validate)) {
}
XmlReader::~XmlReader() throw() { delete m_impl;}


struct XmlUnitializer {
    XercesDOMParser *parser;
    NitroErrorHandler *handler;
    XmlUnitializer() : parser(NULL), handler(NULL) {}
    ~XmlUnitializer() {
        if (parser) delete parser; // should be done before terminate
        if (handler) delete handler;
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
    XmlUnitializer xml_uninitializer;
    
    XercesDOMParser *parser = new XercesDOMParser();
    xml_uninitializer.parser = parser;
    if ( m_impl->m_validate ) {
        parser->setValidationScheme(XercesDOMParser::Val_Always);
        parser->setDoNamespaces(true);    // optional
        parser->setDoSchema(true);
        parser->setValidationSchemaFullChecking(true);
        vector<string> schema_paths;
        string namesp = "http://ubixum.com/deviceinterface/";
        string xsd = "deviceinterface.xsd";

        // add a number of default schema paths

        schema_paths.push_back ( xgetcwd() );
        #ifdef WIN32
        schema_paths.push_back ( xjoin(get_inst_dir(),"xml") );
		#else
		schema_paths.push_back ( "/usr/share/docs/nitro/xml/" );
        #endif
        string schema_path_str;
		size_t pos=0;
		for (vector<string>::iterator itr = schema_paths.begin(); itr != schema_paths.end(); ++itr ) {
			string prefix = "file:///";
			if (itr->substr(0,1) == "/") prefix = "file://";
			string path = xjoin ( *itr, xsd );
			while ( (pos = path.find(" ")) != std::string::npos ) {
				path.replace(pos,1,"%20");
			}
			schema_path_str += namesp + " " + prefix + path + " ";
		}
		parser->setExternalSchemaLocation ( schema_path_str.c_str() );
    }
    parser->setCreateCommentNodes(false);
    parser->setIncludeIgnorableWhitespace(false);


    NitroErrorHandler* handler = new NitroErrorHandler();
    xml_uninitializer.handler = handler;
    parser->setErrorHandler(handler);


    try {
       //DOMDocument *doc = parser->parseURI(m_impl->m_path.c_str()); 
       parser->parse(m_impl->m_path.c_str());
       DOMDocument *doc = parser->getDocument();
       if ( handler->err_occurred() ) {
         throw Exception ( XML_PARSE, handler->get_message() );
       }

       DOMNode* root = doc->getDocumentElement();
       root->normalize(); // gets rid of empty text nodes etc.
       handle_terminals ( node, root, m_impl->m_validate, m_impl->m_path );
       
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

