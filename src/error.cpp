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


#include <nitro/error.h>

#include <iostream>

using namespace std;

namespace Nitro {


string make_str( int32 m_err, const string* m_str, DataType m_userdata );

Exception::Exception(NITRO_ERROR err) : 
#ifdef EXC_USE_STL
 std::runtime_error(make_str(err,NULL,DataType(0))), 
#endif
m_err(err), m_str(NULL),m_userdata(0)
{}

Exception::Exception(int32 err, const std::string& str) : 
#ifdef EXC_USE_STL
 std::runtime_error(make_str(err, &str, DataType(0))), 
#endif
m_err(err), m_str(new string(str)),m_userdata(0) {}

Exception::Exception(int32 err, const std::string& str, const DataType& ud) : 
#ifdef EXC_USE_STL
 std::runtime_error(make_str(err, &str, ud)),
#endif
m_err(err), m_str(new string(str)), m_userdata(ud) {}

Exception::~Exception() throw() {
 if ( m_str ) delete m_str;
}

Exception::Exception(const Exception& e) : 
#ifdef EXC_USE_STL
 std::runtime_error(e.str_error()),
#endif
m_err(e.m_err), m_userdata(e.userdata()) {
    if (e.m_str) { 
        m_str=new string(*e.m_str);
    } else {
        m_str = NULL;
    }
}

string Exception::str_error() const {
    return make_str ( m_err, m_str, m_userdata ); 
}


#define MAKESTR(str) \
    m_str ? string(str) + ": " + *m_str : str

string make_str( int32 m_err, const string* m_str, DataType m_userdata ) {

    if (m_err<0) {
       switch (m_err) {
            case INVALID_CAST:
                return MAKESTR("Invalid Cast between Data Type and native type.");
            case INVALID_TYPE:
                return MAKESTR("Unsupported Data Type");
            case UNSUPPORTED_OP:
                return MAKESTR("Unsupported Operation.");
            case PATH_LOOKUP:
                return MAKESTR("Unable to find absolute path of file.");
                
            case NODE_OP_ERROR:
                return MAKESTR("Invalid node operation.");
            case NODE_NOT_FOUND:
                return MAKESTR("Child node not found.");
            case NODE_DUP_CHILD:
                return MAKESTR("Duplicate child added to node.");
            case NODE_LINKED:
                return MAKESTR("Child node already linked to tree.");
            case NODE_SELFREF:
                return MAKESTR("Nodes cannot reference themselves.");
            case NODE_ATTR_NOT_FOUND: 
                return MAKESTR("Attribute not found.");
            case NODE_UNSUPPORTED_CHILD:
                return MAKESTR("Cannot add incorrect child type to device interface.");
            case NODE_ATTR_ERROR:
                return MAKESTR("Node attribute error.");


            case DEVICE_MUTEX:
                return MAKESTR("Error with device mutex.");
            case DEVICE_OP_ERROR:
                return MAKESTR("Device operation error.");
            case DEVICE_PARSE_ERROR:
                return MAKESTR("Device parse error.");



            case USB_INIT:
                return MAKESTR("Failed to initialize usb interface.");

            case USB_PROTO:
                return MAKESTR("Usb protocol error.");
            case USB_COMM:
                return MAKESTR("Usb communication error.");


            case XML_ENTITY:
                return MAKESTR("Invalid XML Entity.");
            case XML_PARSE:
                return MAKESTR("Error parsing xml.");
            case XML_INVALID:
                return MAKESTR("Invalid XML.");
            case XML_INIT:
                return MAKESTR("Error initializing xml library.");

            case UD_NOT_FOUND:
                return MAKESTR("Dynamic library not found.");
            case UD_NO_FUNC:
                return MAKESTR("User Device missing a function.");
            case UD_PROTO:
                return MAKESTR("User Device function didn't operate as expected.");
            case USB_FIRMWARE:
                return MAKESTR("Invalid USB firmware." );

            case SCRIPTS_INIT:
                return MAKESTR("Cannot initialized script engine.");
            case SCRIPTS_PATH:
                return MAKESTR("Unable to initialize script path.");
            case SCRIPTS_ERR:
                return MAKESTR("Script engine error.");
            case SCRIPTS_SCRIPT:
                return MAKESTR("Error executing user script.");

            default:
                return MAKESTR("Unknown Nitro error.");
        }  
    }

    if (m_str) { return *m_str; }

	return "User Error.";

}


std::ostream& operator << ( std::ostream& stream, const Exception& e) {


    stream << e.str_error() << " (" << e.code() << ')';
    return stream;
}


} // end namespace
