// Copyright (C) 2009 Ubixum, Inc. 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef NITRO_ERROR_H
#define NITRO_ERROR_H

#include <iosfwd>

#include "types.h" // DLL_API

namespace Nitro {


/**
 *  \brief Nitro Error codes
 *
 *  By convention, Nitro error codes are negative.
 *  Other functions that throw a NitroException should use
 *  a positive code and and error string.
 **/
enum NITRO_ERROR {


    INVALID_TYPE=-1, ///< Unsupported data type. 
    INVALID_CAST=-2, ///< Invalid cast between DataType and native type.
    UNSUPPORTED_OP=-3, ///< Unsupported Operation

    NODE_OP_ERROR=-50, ///< Node Operation error
    NODE_NOT_FOUND, ///< Child node not found
    NODE_DUP_CHILD, ///< Duplicate child node addition.
    NODE_UNSUPPORTED_CHILD, ///< Incorrect child type added to node.
    NODE_LINKED, ///< Child node already linked in tree
    NODE_SELFREF, ///< Node added itself to children.
    NODE_ATTR_NOT_FOUND, ///< Node attribute not found.
    NODE_ATTR_ERROR, ///< Node attribute error.

    DEVICE_MUTEX=-100, ///< Device Mutex Error
    DEVICE_OP_ERROR, ///< Device operation error
    DEVICE_PARSE_ERROR, ///< Error parsing device get/set register

    XML_ENTITY=-150, ///< Invalid XML Entity
    XML_PARSE, ///< XML Parse Error.
    XML_INVALID, ///< Invalid XML data.
    XML_INIT, ///< Error initializing XML library.

    UD_NOT_FOUND=-200, ///< User device library not found
    UD_NO_FUNC, ///< Library missing a function.
    UD_PROTO, ///< User device function didn't operate as expected.

    SCRIPTS_INIT=-250, ///< Unable to initialize script engine
    SCRIPTS_PATH, ///< Unable to initialize script path
    SCRIPTS_ERR, ///< Script engine error.
    SCRIPTS_SCRIPT, ///< Error with user script.

    USB_INIT=-300, ///< Usb initialization error.
    USB_PROTO, ///< Usb protocol error.
    USB_COMM, ///< Usb communication error.
    USB_FIRMWARE ///< Invalid USB Firmware
};


/**
 * \brief %Exception class thrown by most %Nitro methods.
 **/
class DLL_API Exception {

    friend DLL_API std::ostream& operator << (std::ostream&, const Exception& );
    private:
        
        /***
         * Not Implemented assignment operator
         **/
         Exception& operator == ( const Exception& e ); 
    
    protected:
        int32 m_err;
        const std::string* m_str;
        DataType m_userdata;
    public:
        /**
         * \brief Construct an Exception from an error code.
         *
         * \param err The Nitro error code
         **/
         Exception (NITRO_ERROR err);

        /**
         * \brief Construct an Exception from an error code and string.
         *
         * The string explanation of Nitro Error codes is looked up 
         * at runtime based on the value of err.  If err is negative, it 
         * is assumed to be a NITRO_ERROR. In this case, the error msg
         * will use the be appended to a NITRO_ERROR string as supporting
         * information.  If err is positive str_error will be printed 
         * without modification.
         *
         * \param err Error code.  Positive for user errors, negative should be 
         *      a NITRO_ERROR
         * \param str_error Supporting Error information.
         **/
        Exception(int32 err, const std::string& str_error);


        /**
         * \brief Construct an Exception with user data.
         * 
         * An application can pass optional data to the calling scope with
         * the userdata parameter.
         **/
        Exception(int32 err, const std::string& str_error, const DataType& userdata );


        /**
         * Copy constructor. 
         **/
        Exception ( const Exception& e );
        
        virtual ~Exception() throw();


        /**
         * \brief Return the error code for this exception.
         **/
        int32 code() const { return m_err; }

        /**
         * \brief Return the error string associated with this exception.
         **/
        std::string str_error() const;


        /**
         * \brief Retrieve userdata associated with exception.
         **/
        DataType userdata() const { return m_userdata; }
        
};

/**
 * Prints the Nitro::Exception to the ostream.
 **/
DLL_API std::ostream& operator << (std::ostream& stream, const Exception& e);


} // end namespace

#endif 
