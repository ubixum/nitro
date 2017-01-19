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
#ifndef NITRO_TYPES_H
#define NITRO_TYPES_H


#include <string>
#include <vector>
#include <iosfwd>

#ifdef WIN32
#include <BaseTsd.h> // Windows types
typedef UINT64 uint64;
typedef INT64 int64;
typedef UINT32 uint32;
typedef INT32 int32;
typedef UINT16 uint16;
typedef INT16 int16;
typedef UINT8 uint8;
typedef INT8 int8;

#ifdef NITRO_STATIC
#define DLL_API
#else
#ifdef DRIVER_EXPORTS
 #define DLL_API __declspec(dllexport)
#else
 #define DLL_API __declspec(dllimport)
#endif
#endif


#else /* for Linux */
#define DLL_API 
#include <stddef.h> // size_t
#include <stdint.h> // uint types

typedef uint16_t uint16;
typedef int16_t int16;
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
#endif


/**
 * \brief Use the Nitro namespace for all Nitro API classes/functions.
 **/
namespace Nitro {

typedef enum {
	//UNINIT_DATA, // uninitialized data
    INT_DATA, ///< Integer Data Type
    UINT_DATA, ///< Unsigned Integer Data
    BIGINT_DATA, ///< for int types wider than 32 bits
    FLOAT_DATA, ///< double or float equivalent
    STR_DATA, ///< String data type, represented by std::string
    NODE_DATA,///< DataType is a NodeRef
    LIST_DATA, ///< DataType is std::vector<DataType>
    DEV_DATA, ///< DataType is a Device
	BUF_DATA ///< raw buffer.  In C++, this is unsigned char[]
} DATA_TYPE;


class NodeRef;
class Device;


/**
 * \brief DataType serves as an abstraction of native integer addresses and other datatypes.
 *
 * Currently, integer, unsigned integer, and string data types are supported.  DataType
 * is immutable.
 * 
 * Nitro::Device methods that take DataType parameters can be made with native integer
 * or string parameters.
 * \code
 *  Device.set("terminal_name", 1, 5); // set register 1 of "terminal_name" to 5.
 * \endcode
 **/
class DLL_API DataType {
    private:
       DATA_TYPE m_type;
       union {
            int32 m_int;
            uint32 m_uint;
            double m_float;
       }; 
       std::string m_str;
       NodeRef* m_node;
       std::vector<DataType>* m_list;
       Device* m_dev;
	   void copy(const DataType& other);
       uint8* m_buf;
    public:

		/**
		 * \brief Construct a buffer datatype
		 * 
		 * Currently, this is the only way to construct a buffer datatype.
		 * This is because the constructors for char* construct a string datatype
		 **/
		static DataType as_datatype(uint8* buf, uint32 len);

		/**
		 * \brief Return a datatype to buffer form.
         *
         * Cast datatype back to raw buffer.
         * The length of the buffer can be obtained by casting the datatype to a uint32
		 **/
		static uint8* as_buffer(const DataType& d);


        /**
         * \brief for bigint data types
         **/
        static DataType as_bigint_datatype ( const std::vector<DataType> &ints , const std::string str="");

        /**
         * \brief for bigint data types
         **/
        static std::vector<DataType> as_bigints ( const DataType& d );

        /**
         * \brief Construct an integer DataType.
         **/
        DataType(int32 value);
        /**
         * \brief Construct an unsigned integer DataType.
         **/
        DataType(uint32 value);

        /**
         * \brief Construct a float DataType
         **/
        DataType(double value);

        /**
         * \brief Construct a string DataType.  Commonly used for terminal and register names.
         **/
        DataType(const std::string& value);
        /**
         * \brief Construct a string DataType.
         **/
        DataType(const char* value);

        /**
         * \brief Construct a new DataType with same type/value as other.
         * \param other DataType to copy.
         **/
        DataType(const DataType& other);

        /**
         * \brief Construct a new DataType with a NodeRef
         **/
        DataType(const NodeRef& node);

        /**
         * \brief Construct a new DataType for a DataType vector
         **/
        DataType(const std::vector<DataType>& list);

        /**
         * \brief Construct a new DataType from a Device
         *
         * Devices are not references counted.  The DataType
         * only stores a pointer to the device.  The Device must 
         * main valid as long as the DataType is in use.
         **/
        DataType(Device& dev);


        /**
         * \brief DataType destructor
         **/
        ~DataType() throw();

        /**
         * \brief Assignment operator.
         **/
		DataType& operator=(const DataType&);

        /**
         * \brief value comparision for two data types.
         *
         * Implements value comparision.  Two DataTypes are
         * equal if a.value == b.value.
         * Note that types are ignored for integers but are considered
         * in the case of integer to string comparisions.  (Integer types
         * are never equal to string types.)
         **/
        bool operator==(const DataType&) const;
        /**
         * \see operator==()
         **/
        bool operator!=(const DataType&) const;

        /**
         * Casts T to DataType in order to correctly call ==.
         **/
        template<typename T>
        bool operator==(const T& t) const { return *this == DataType(t); }
        template<typename T>
        bool operator!=(const T& t) const { return *this != DataType(t); }
        /**
         * \brief string representation of underlying data type.
         * \return "int", "uint", or "str"
         **/
        const std::string& str_type() const;
        /**
         * \brief returns underlying DATA_TYPE.
         * \return Nitro::DATA_TYPE
         **/
        DATA_TYPE get_type() const ; 
        /**
         * \brief string representation of underlying value.
         * \return String values are returned as they are stored.  int/uint
         * values are converted to a string representation.
         **/
        const std::string& str_value() const; 

        /**
         * \brief Cast DataType to int32
         *
         * DataType can be explicitly cast to an int32.
         * Trying to cast a string or node to an int is undefined.  (It currently doesn't parse an integer.)
         **/
        operator int32() const;
        /**
         * \brief Cast DataType to uint32.
         *
         * DataType can be explicitly cast to a uint32.  Behavior for string and nodes are the same as for int32 operator.
         **/
        operator uint32() const;

        /**
         * \brief Cast DataType to double
         **/
        operator double() const;

        /**
         * \brief Cast DataType to string.
         *
         * Unlike the int32 operator, this operator does return a string
         * representation of the DataType even for integer or node data types.
         **/
        operator std::string() const;


        /**
         * \brief Cast DataType to Node
         *
         * \throw Exception if DATA_TYPE is not NODE_DATA
         **/
        operator NodeRef () const;


        /**
         * \brief Cast DataType to vector<DataType>
         *
         * \throw Exception if DATA_TYPE is not NODE_DATA
         **/
        operator std::vector<DataType> () const;


        /**
         * \brief Cast DataType to a Device
         *
         * \throw Exception if DATA_TYPE is not DEV_DATA
         **/
         operator Device& () const;

};


/**
 * DataTypes are printed with the Nitro::DataType::str_value().
 **/
DLL_API std::ostream& operator << ( std::ostream& stream, const DataType& data );

/**
 * \brief print a list of datatypes
 **/
DLL_API std::ostream& operator << ( std::ostream& stream, const std::vector<DataType>& list );



} // end namespace

#endif
